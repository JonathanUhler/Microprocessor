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
    ADDI = 0b000010,
    SUBI = 0b000110,
    ANDI = 0b001010,
    ORI  = 0b001110,
    XORI = 0b010010,
    SLLI = 0b010110,
    SRLI = 0b011010,
    SRAI = 0b011110,
    LD   = 0b110010,
    ST   = 0b110110,
    JLZ  = 0b111010,
    JLO  = 0b111110,

    // Fourth row (DSS-Type)
    ADD  = 0b000011,
    SUB  = 0b000111,
    AND  = 0b001011,
    OR   = 0b001111,
    XOR  = 0b010011,
    SLL  = 0b010111,
    SRL  = 0b011011,
    SRA  = 0b011111,
    EQ   = 0b100011,
    GT   = 0b100111,
    LT   = 0b101011,
    NE   = 0b101111,
    JLRZ = 0b111011,
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


struct parser_group_node *parser_parse_file(FILE *file, uint16_t base_address);


void parser_free_group_nodes(struct parser_group_node *head);


#endif  // _ASSEMBLER_PARSER_H_
