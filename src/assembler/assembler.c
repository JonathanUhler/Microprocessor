#include "assembler/encoder.h"
#include "assembler/lexer.h"
#include "assembler/parser.h"
#include "architecture/logger.h"
#include "structures/list.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>


void usage(const char *error) {
    if (error != NULL) {
        log_error("%s", error);
    }

    printf("usage: assembler [-o path] [-v] path\n");
    printf("\n");
    printf("options:\n");
    printf("  -o path  specify the output path for the generated binary (default a.out)\n");
    printf("  -v       verbosity level for log messages, can be specified multiple times\n");
    printf("\n");
    printf("argument:\n");
    printf("  path     the path to the assembly source file\n");
    exit(error != NULL);
}


int main(int argc, char *argv[]) {
    char *output_path = "./a.out";
    char *input_path = NULL;
    enum logger_log_level verbosity = LOGGER_LEVEL_WARN;

    int flag;
    while ((flag = getopt(argc, argv, "o:v")) != -1) {
        switch (flag) {
        case 'o':
            output_path = optarg;
            break;
        case 'v':
            verbosity++;
            break;
        default:
            usage("unknown option flag");
            break;
        }
    }

    logger_set_level(verbosity);

    if (optind >= argc || argv[optind] == NULL) {
        usage("missing required 'path' argument");
    }
    input_path = argv[optind];

    FILE *in_file = fopen(input_path, "r");
    if (in_file == NULL) {
        log_fatal("Cannot open input file '%s'", input_path);
    }

    struct list *tokens;
    enum lexer_status lex_status = lexer_lex_file(in_file, &tokens);
    if (lex_status != LEXER_STATUS_SUCCESS) {
        log_fatal("Lexer failed, will not proceed with parsing (errno %d)", lex_status);
    }

    struct list *groups;
    enum parser_status parse_status = parser_parse_tokens(tokens, &groups);
    if (parse_status != PARSER_STATUS_SUCCESS) {
        log_fatal("Parser failed, will not proceed with encoding (errno %d)", parse_status);
    }

    struct list *bytes;
    enum encoder_status encoder_status = encoder_encode_groups(groups, &bytes);
    if (encoder_status != ENCODER_STATUS_SUCCESS) {
        log_fatal("Encoder failed, will not proceed with output file writing");
    }

    FILE *out_file = fopen(output_path, "wb");
    if (out_file == NULL) {
        log_fatal("Cannot open output file '%s'", output_path);
    }

    for (uint32_t i = 0; i < bytes->size; i++) {
        void *data;
        list_peek_at(bytes, i, &data);
        uint8_t *byte = (uint8_t *) data;
        if (fwrite(byte, sizeof(uint8_t), 1, out_file) != 1) {
            log_fatal("Cannot write to output file '%s'", output_path);
        }
    }

    fclose(in_file);
    fclose(out_file);
    destroy_list(tokens, &list_default_node_free_callback);
    destroy_list(groups, &list_default_node_free_callback);
    destroy_list(bytes, &list_default_node_free_callback);
    return 0;
}
