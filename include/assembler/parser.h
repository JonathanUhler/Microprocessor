#ifndef _ASSEMBLER_PARSER_H_
#define _ASSEMBLER_PARSER_H_


#include "assembler/lexer.h"
#include <stdint.h>
#include <stdio.h>


#define PARSER_OPCODE_NAME_MAX_LENGTH 7

#define PARSER_FORMAT_SIZE     2
#define PARSER_FUNCT_SIZE      4
#define PARSER_REGISTER_SIZE   5
#define PARSER_IMMEDIATE_SIZE 16


struct parser_i_format {
    uint16_t format       : PARSER_FORMAT_SIZE;
    uint16_t funct        : PARSER_FUNCT_SIZE;
    uint16_t __RESERVED__ : 10;
    uint16_t immediate    : PARSER_IMMEDIATE_SIZE;
} __attribute__((packed));


struct parser_dsi_format {
    uint16_t format    : PARSER_FORMAT_SIZE;
    uint16_t funct     : PARSER_FUNCT_SIZE;
    uint16_t dest      : PARSER_REGISTER_SIZE;
    uint16_t source1   : PARSER_REGISTER_SIZE;
    uint16_t immediate : PARSER_IMMEDIATE_SIZE;
} __attribute__((packed));


struct parser_dss_format {
    uint16_t format       : PARSER_FORMAT_SIZE;
    uint16_t funct        : PARSER_FUNCT_SIZE;
    uint16_t dest         : PARSER_REGISTER_SIZE;
    uint16_t source1      : PARSER_REGISTER_SIZE;
    uint16_t source2      : PARSER_REGISTER_SIZE;
    uint16_t __RESERVED__ : 11;
} __attribute__((packed));


union parser_instruction_format {
    struct parser_i_format i_type;
    struct parser_dsi_format dsi_type;
    struct parser_dss_format dss_type;
    uint32_t binary;
};


enum parser_opcode {
    // First row (I-Type)
    HALT = 0b000000,

    // Third row (DSI-Type)
    ADDI = 0b100000,
    SUBI = 0b100001,
    ANDI = 0b100010,
    ORI  = 0b100011,
    XORI = 0b100100,
    SLLI = 0b100101,
    SRLI = 0b100110,
    SRAI = 0b100111,
    LD   = 0b101100,
    ST   = 0b101101,
    JLZ  = 0b101110,
    JLO  = 0b101111,

    // Fourth row (DSS-Type)
    ADD  = 0b110000,
    SUB  = 0b110001,
    AND  = 0b110010,
    OR   = 0b110011,
    XOR  = 0b110100,
    SLL  = 0b110101,
    SRL  = 0b110110,
    SRA  = 0b110111,
    EQ   = 0b111000,
    GT   = 0b111001,
    LT   = 0b111010,
    NE   = 0b111011,
    JLRZ = 0b111110,
    JLRO = 0b111111
};


enum parser_opcode_format {
    PARSER_OPCODE_FORMAT_I      = 0b00,
    PARSER_OPCODE_FORMAT_DSI    = 0b10,
    PARSER_OPCODE_FORMAT_DSS    = 0b11,
    PARSER_OPCODE_FORMAT_PSEUDO = 0xFF
};


struct parser_opcode_name {
    const char *name;
    enum parser_opcode opcode;
    enum parser_opcode_format format;
    uint32_t funct;
};


enum parser_group_type {
    PARSER_GROUP_INSTRUCTION,
    PARSER_GROUP_LABEL,
    PARSER_GROUP_EOF
};


struct parser_group {
    enum parser_group_type type;
    enum parser_opcode opcode;
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


const struct parser_opcode_name *parser_opcode_name_to_value(const char *name);


const struct parser_opcode_name *parser_opcode_value_to_name(enum parser_opcode value);


enum parser_status parser_next_group(FILE *file,
                                     struct parser_group *group,
                                     struct lexer_token *last_token);


struct parser_group_node *parser_parse_file(FILE *file);


void parser_free_group_nodes(struct parser_group_node *head);


#endif  // _ASSEMBLER_PARSER_H_
