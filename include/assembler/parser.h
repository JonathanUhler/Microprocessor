#ifndef _ASSEMBLER_PARSER_H_
#define _ASSEMBLER_PARSER_H_


#include "assembler/lexer.h"
#include "architecture/isa.h"
#include <stdint.h>
#include <stdio.h>


enum parser_group_type {
    PARSER_GROUP_INSTRUCTION,
    PARSER_GROUP_LABEL,
    PARSER_GROUP_EOF
};


struct parser_group {
    enum parser_group_type type;
    enum isa_opcode opcode;
    uint32_t rd;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t imm_num;
    char imm_label[LEXER_TOKEN_MAX_LENGTH + 1];
};


struct parser_group_node {
    struct parser_group group;
    uint32_t binary;
    struct parser_group_node *next;
};


enum parser_status {
    PARSER_STATUS_SUCCESS = 0,
    PARSER_STATUS_EOF     = 1,
    PARSER_STATUS_INVALID_ARGUMENT,
    PARSER_STATUS_SEMANTIC_ERROR
};


enum parser_status parser_next_group(FILE *file,
                                     struct parser_group *group,
                                     struct lexer_token *last_token);


struct parser_group_node *parser_parse_file(FILE *file, uint16_t base_address);


void parser_free_group_nodes(struct parser_group_node *head);


#endif  // _ASSEMBLER_PARSER_H_
