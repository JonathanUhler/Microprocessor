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
    printf("  -b addr  specify the base address to place code (default 0x0100)\n");
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
    uint16_t base_address = 0x0100;
    enum logger_log_level verbosity = LOGGER_LEVEL_WARN;

    int flag;
    while ((flag = getopt(argc, argv, "b:o:v")) != -1) {
        switch (flag) {
        case 'b':
            base_address = atoi(optarg);
            break;
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
        log_fatal("cannot open input file '%s'", input_path);
    }

    struct list *tokens;
    enum lexer_status lex_status = lexer_lex_file(in_file, &tokens);
    if (lex_status != LEXER_STATUS_SUCCESS) {
        log_fatal("lexer failed, will not proceed with parsing (errno %d)", lex_status);
    }

    struct list *groups;
    enum parser_status parse_status = parser_parse_tokens(tokens, base_address, &groups);
    if (parse_status != PARSER_STATUS_SUCCESS) {
        log_fatal("parser failed, will not proceed with encoding (errno %d)", parse_status);
    }

    enum encoder_status encoder_status = encoder_encode_groups(groups);
    if (encoder_status != ENCODER_STATUS_SUCCESS) {
        log_fatal("encoder failed, will not proceed with output file writing");
    }

    FILE *out_file = fopen(output_path, "wb");
    if (out_file == NULL) {
        log_fatal("cannot open output file '%s'", output_path);
    }

    for (uint32_t i = 0; i < groups->size; i++) {
        void *data;
        list_peek_at(groups, i, &data);
        struct parser_group *group = (struct parser_group *) data;

        uint8_t bytes[4] = {
            (group->instruction.binary >>  0) & 0xFF,
            (group->instruction.binary >>  8) & 0xFF,
            (group->instruction.binary >> 16) & 0xFF,
            (group->instruction.binary >> 24) & 0xFF
        };

        if (fwrite(bytes, sizeof(bytes), 1, out_file) != 1) {
            log_fatal("cannot write to output file '%s'", output_path);
        }
    }

    fclose(in_file);
    fclose(out_file);
    destroy_list(tokens, &list_default_node_free_callback);
    destroy_list(groups, &list_default_node_free_callback);
    return 0;
}
