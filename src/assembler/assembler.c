#include "assembler/encoder.h"
#include "assembler/lexer.h"
#include "assembler/parser.h"
#include "architecture/logger.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>


void usage(const char *error) {
    if (error != NULL) {
        log_error("%s", error);
    }

    printf("usage: assembler [-o path] path\n");
    printf("\n");
    printf("options:\n");
    printf("  -b addr  specify the base address to place code (default 0x0100)\n");
    printf("  -o path  specify the output path for the generated binary (default a.out)\n");
    printf("\n");
    printf("argument:\n");
    printf("  path     the path to the assembly source file\n");
    exit(error != NULL);
}


int main(int argc, char *argv[]) {
    char *output_path = "./a.out";
    char *input_path = NULL;
    uint16_t base_address = 0x0100;

    int flag;
    while ((flag = getopt(argc, argv, "b:o:")) != -1) {
        switch (flag) {
        case 'b':
            base_address = atoi(optarg);
            break;
        case 'o':
            output_path = optarg;
            break;
        default:
            usage("unknown option flag");
            break;
        }
    }

    if (optind >= argc || argv[optind] == NULL) {
        usage("missing required 'path' argument");
    }
    input_path = argv[optind];

    FILE *in_file = fopen(input_path, "r");
    if (in_file == NULL) {
        log_fatal("cannot open input file '%s'", input_path);
    }

    struct parser_group_node *groups = parser_parse_file(in_file, base_address);
    if (groups == NULL) {
        log_fatal("parser failed, will not proceed with encoding");
    }

    enum encoder_status encoder_status = encoder_encode_groups(&groups);
    if (encoder_status != ENCODER_STATUS_SUCCESS) {
        log_fatal("encoder failed, will not proceed with output file writing");
    }

    FILE *out_file = fopen(output_path, "wb");
    if (out_file == NULL) {
        fclose(in_file);
        parser_free_group_nodes(groups);
        log_fatal("cannot open output file '%s'", output_path);
    }

    struct parser_group_node *curr = groups;
    while (curr != NULL) {
        uint8_t bytes[4] = {
            (curr->binary >>  0) & 0xFF,
            (curr->binary >>  8) & 0xFF,
            (curr->binary >> 16) & 0xFF,
            (curr->binary >> 24) & 0xFF
        };
        if (fwrite(bytes, sizeof(bytes), 1, out_file) != 1) {
            fclose(in_file);
            fclose(out_file);
            parser_free_group_nodes(groups);
            log_fatal("cannot write to output file '%s'", output_path);
        }
        curr = curr->next;
    }

    fclose(in_file);
    fclose(out_file);
    parser_free_group_nodes(groups);
    return 0;
}
