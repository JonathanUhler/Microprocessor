#include "assembler/parser.h"
#include "assembler/lexer.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


static const struct parser_opcode_name parser_opcode_table[] = {
    // First row (I-Type)
    {"halt", HALT, HALT >> PARSER_FUNCT_SIZE, HALT & (1U << PARSER_FUNCT_SIZE)},
    
    // Third row (DSI-Type)
    {"addi", ADDI, ADDI >> PARSER_FUNCT_SIZE, ADDI & (1U << PARSER_FUNCT_SIZE)},
    {"subi", SUBI, SUBI >> PARSER_FUNCT_SIZE, SUBI & (1U << PARSER_FUNCT_SIZE)},
    {"andi", ANDI, ANDI >> PARSER_FUNCT_SIZE, ANDI & (1U << PARSER_FUNCT_SIZE)},
    {"ori", ORI, ORI >> PARSER_FUNCT_SIZE, ORI & (1U << PARSER_FUNCT_SIZE)},
    {"xori", XORI, XORI >> PARSER_FUNCT_SIZE, XORI & (1U << PARSER_FUNCT_SIZE)},
    {"slli", SLLI, SLLI >> PARSER_FUNCT_SIZE, SLLI & (1U << PARSER_FUNCT_SIZE)},
    {"srli", SRLI, SRLI >> PARSER_FUNCT_SIZE, SRLI & (1U << PARSER_FUNCT_SIZE)},
    {"srai", SRAI, SRAI >> PARSER_FUNCT_SIZE, SRAI & (1U << PARSER_FUNCT_SIZE)},
    {"ld", LD, LD >> PARSER_FUNCT_SIZE, LD & (1U << PARSER_FUNCT_SIZE)},
    {"st", ST, ST >> PARSER_FUNCT_SIZE, ST & (1U << PARSER_FUNCT_SIZE)},
    {"jlz", JLZ, JLZ >> PARSER_FUNCT_SIZE, JLZ & (1U << PARSER_FUNCT_SIZE)},
    {"jlo", JLO, JLO >> PARSER_FUNCT_SIZE, JLO & (1U << PARSER_FUNCT_SIZE)},
    
    // Fourth row (DSS-Type)
    {"add", ADD, ADD >> PARSER_FUNCT_SIZE, ADD & (1U << PARSER_FUNCT_SIZE)},
    {"sub", SUB, SUB >> PARSER_FUNCT_SIZE, SUB & (1U << PARSER_FUNCT_SIZE)},
    {"and", AND, AND >> PARSER_FUNCT_SIZE, AND & (1U << PARSER_FUNCT_SIZE)},
    {"or", OR, OR >> PARSER_FUNCT_SIZE, OR & (1U << PARSER_FUNCT_SIZE)},
    {"xor", XOR, XOR >> PARSER_FUNCT_SIZE, XOR & (1U << PARSER_FUNCT_SIZE)},
    {"sll", SLL, SLL >> PARSER_FUNCT_SIZE, SLL & (1U << PARSER_FUNCT_SIZE)},
    {"srl", SRL, SRL >> PARSER_FUNCT_SIZE, SRL & (1U << PARSER_FUNCT_SIZE)},
    {"sra", SRA, SRA >> PARSER_FUNCT_SIZE, SRA & (1U << PARSER_FUNCT_SIZE)},
    {"eq", EQ, EQ >> PARSER_FUNCT_SIZE, EQ & (1U << PARSER_FUNCT_SIZE)},
    {"gt", GT, GT >> PARSER_FUNCT_SIZE, GT & (1U << PARSER_FUNCT_SIZE)},
    {"lt", LT, LT >> PARSER_FUNCT_SIZE, LT & (1U << PARSER_FUNCT_SIZE)},
    {"ne", NE, NE >> PARSER_FUNCT_SIZE, NE & (1U << PARSER_FUNCT_SIZE)},
    {"jlrz", JLRZ, JLRZ >> PARSER_FUNCT_SIZE, JLRZ & (1U << PARSER_FUNCT_SIZE)},
    {"jlro", JLRO, JLRO >> PARSER_FUNCT_SIZE, JLRO & (1U << PARSER_FUNCT_SIZE)},
    
    // Pseudo instructions
    {"j", JLZ, JLZ >> PARSER_FUNCT_SIZE, JLZ & (1U << PARSER_FUNCT_SIZE)},
    {"jl", JLZ, JLZ >> PARSER_FUNCT_SIZE, JLZ & (1U << PARSER_FUNCT_SIZE)},
    {"jlr", JLRZ, JLRZ >> PARSER_FUNCT_SIZE, JLRZ & (1U << PARSER_FUNCT_SIZE)},
    {"jo", JLO, JLO >> PARSER_FUNCT_SIZE, JLO & (1U << PARSER_FUNCT_SIZE)},
    {"jz", JLZ, JLZ >> PARSER_FUNCT_SIZE, JLZ & (1U << PARSER_FUNCT_SIZE)},
    {"call", JLZ, JLZ >> PARSER_FUNCT_SIZE, JLZ & (1U << PARSER_FUNCT_SIZE)},
    {"li", ORI, ORI >> PARSER_FUNCT_SIZE, ORI & (1U << PARSER_FUNCT_SIZE)},
    {"mv", OR, OR >> PARSER_FUNCT_SIZE, OR & (1U << PARSER_FUNCT_SIZE)},
    {"nop", OR, OR >> PARSER_FUNCT_SIZE, OR & (1U << PARSER_FUNCT_SIZE)},
    {"ret", JLRZ, JLRZ >> PARSER_FUNCT_SIZE, JLRZ & (1U << PARSER_FUNCT_SIZE)}
};


const struct parser_opcode_name *parser_opcode_name_to_value(const char *name) {
    size_t n = sizeof(parser_opcode_table) / sizeof(parser_opcode_table[0]);

    for (size_t i = 0; i < n; i++) {
        if (strncmp(name, parser_opcode_table[i].name, PARSER_OPCODE_NAME_MAX_LENGTH) == 0) {
            return &parser_opcode_table[i];
        }
    }

    return NULL;
}


static enum parser_status parser_expect_register(FILE *file, struct lexer_token *token) {
    enum lexer_status lex_status = lexer_next_token(file, token);
    if (lex_status != LEXER_STATUS_SUCCESS || token->type != LEXER_TOKEN_REGISTER) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_immediate(FILE *file, struct lexer_token *token) {
    enum lexer_status lex_status = lexer_next_token(file, token);
    if (lex_status != LEXER_STATUS_SUCCESS) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
    if (token->type != LEXER_TOKEN_IDENTIFIER && token->type != LEXER_TOKEN_NUMBER) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_comma(FILE *file, struct lexer_token *token) {
    enum lexer_status lex_status = lexer_next_token(file, token);
    if (lex_status != LEXER_STATUS_SUCCESS || token->type != LEXER_TOKEN_COMMA) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_colon(FILE *file, struct lexer_token *token) {
    enum lexer_status lex_status = lexer_next_token(file, token);
    if (lex_status != LEXER_STATUS_SUCCESS || token->type != LEXER_TOKEN_COLON) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
    return PARSER_STATUS_SUCCESS;
}



static enum parser_status parser_expect_i_instruction(FILE *file,
                                                      struct parser_group *group,
                                                      struct lexer_token *token)
{
    enum parser_status parse_status;
        
    if ((parse_status = parser_expect_immediate(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }
    switch (token->type) {
    case LEXER_TOKEN_IDENTIFIER:
        strncpy(group->imm_label, token->text, LEXER_TOKEN_MAX_LENGTH);
        group->imm_label[LEXER_TOKEN_MAX_LENGTH] = '\0';
        break;
    case LEXER_TOKEN_NUMBER:
        group->imm_num = token->value;
        break;
    default:
        break;  // Unreachable
    }

    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_dsi_instruction(FILE *file,
                                                        struct parser_group *group,
                                                        struct lexer_token *token)
{
    enum parser_status parse_status;

    if ((parse_status = parser_expect_register(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }
    group->rd = token->value;

    if ((parse_status = parser_expect_comma(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }

    if ((parse_status = parser_expect_register(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }
    group->rs1 = token->value;

    if ((parse_status = parser_expect_comma(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }

    if ((parse_status = parser_expect_immediate(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }
    switch (token->type) {
    case LEXER_TOKEN_IDENTIFIER:
        strncpy(group->imm_label, token->text, LEXER_TOKEN_MAX_LENGTH);
        group->imm_label[LEXER_TOKEN_MAX_LENGTH] = '\0';
        break;
    case LEXER_TOKEN_NUMBER:
        group->imm_num = token->value;
        break;
    default:
        break;  // Unreachable
    }

    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_dss_instruction(FILE *file,
                                                        struct parser_group *group,
                                                        struct lexer_token *token)
{
    enum parser_status parse_status;

    if ((parse_status = parser_expect_register(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }
    group->rd = token->value;

    if ((parse_status = parser_expect_comma(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }

    if ((parse_status = parser_expect_register(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }
    group->rs1 = token->value;

    if ((parse_status = parser_expect_comma(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }

    if ((parse_status = parser_expect_register(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }
    group->rs2 = token->value;

    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_instruction(FILE *file,
                                                    struct parser_group *group,
                                                    struct lexer_token *token)
{
    const struct parser_opcode_name *opcode_name = parser_opcode_name_to_value(token->text);

    // TODO: handle pseudo instructions

    group->opcode = opcode_name->opcode;

    if (opcode_name->format == PARSER_OPCODE_FORMAT_I) {
        return parser_expect_i_instruction(file, group, token);
    }
    else if (opcode_name->format == PARSER_OPCODE_FORMAT_DSI) {
        return parser_expect_dsi_instruction(file, group, token);
    }
    else if (opcode_name->format == PARSER_OPCODE_FORMAT_DSS) {
        return parser_expect_dss_instruction(file, group, token);
    }
    else {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
}


static enum parser_status parser_expect_label(FILE *file,
                                              struct parser_group *group,
                                              struct lexer_token *token)
{
    enum parser_status parse_status;

    strncpy(group->imm_label, token->text, LEXER_TOKEN_MAX_LENGTH);
    group->imm_label[LEXER_TOKEN_MAX_LENGTH] = '\0';

    if ((parse_status = parser_expect_colon(file, token)) != PARSER_STATUS_SUCCESS) {
        return parse_status;
    }

    return PARSER_STATUS_SUCCESS;
}


enum parser_status parser_next_group(FILE *file,
                                     struct parser_group *group,
                                     struct lexer_token *token)
{
    if (file == NULL || group == NULL) {
        return PARSER_STATUS_INVALID_ARGUMENT;
    }

    group->opcode = 0;
    group->rd = 0;
    group->rs1 = 0;
    group->rs2 = 0;
    group->imm_num = 0;
    memset(&group->imm_label, 0, sizeof(group->imm_label));

    if (token == NULL) {
        struct lexer_token local_token = {0};
        token = &local_token;
    }

    enum lexer_status lex_status = lexer_next_token(file, token);
    switch (lex_status) {
    case LEXER_STATUS_SUCCESS:
        break;  // Good
    case LEXER_STATUS_EOF:
        return PARSER_STATUS_EOF;
    default:
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    if (token->type != LEXER_TOKEN_IDENTIFIER) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    const struct parser_opcode_name *opcode_name = parser_opcode_name_to_value(token->text);
    if (opcode_name != NULL) {
        return parser_expect_instruction(file, group, token);
    }
    else {
        return parser_expect_label(file, group, token);
    }
}
