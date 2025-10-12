#ifndef _ASSEMBLER_PARSER_H_
#define _ASSEMBLER_PARSER_H_


#include "assembler/lexer.h"


#define PARSER_OPCODE_NAME_MAX_LENGTH 7


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


struct parser_opcode_name {
    const char *name;
    enum parser_opcode opcode;
};


struct parser_group {
    enum parser_opcode opcode;
    uint32_t rd;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t imm_num;
    char imm_label[LEXER_TOKEN_MAX_LENGTH + 1];
};


enum parser_status {
    PARSER_STATUS_SUCCESS = 0,
    PARSER_STATUS_EOF     = 1,
    PARSER_STATUS_INVALID_ARGUMENT,
    PARSER_STATUS_SEMANTIC_ERROR
};


const struct parser_opcode_name *parser_opcode_name_to_value(const char *name);


enum parser_status parser_next_group(FILE *file,
                                     struct parser_group *group,
                                     struct lexer_token *last_token);


/*
  PATTERNS:

  Label: IDENT COLON

  I-Type: IDENT [NUMBER | IDENT]

  DSI-Type: IDENT REGISTER COMMA REGISTER COMMA [NUMBER | IDENT]

  DSS-Type: IDENT REGISTER COMMA REGISTER COMMA REGISTER

  End: EOF
 */


#endif  // _ASSEMBLER_PARSER_H_
