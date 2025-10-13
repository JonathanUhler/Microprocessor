#include "assembler/parser.h"
#include "assembler/lexer.h"
#include "assembler/logger.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


static uint32_t parser_instruction_count = 0;


static const struct parser_opcode_name parser_opcode_table[] = {
    // First row (I-Type)
    {"halt", HALT, HALT & ((1U << PARSER_FORMAT_SIZE) - 1U), HALT >> PARSER_FORMAT_SIZE},
    
    // Third row (DSI-Type)
    {"addi", ADDI, ADDI & ((1U << PARSER_FORMAT_SIZE) - 1U), ADDI >> PARSER_FORMAT_SIZE},
    {"subi", SUBI, SUBI & ((1U << PARSER_FORMAT_SIZE) - 1U), SUBI >> PARSER_FORMAT_SIZE},
    {"andi", ANDI, ANDI & ((1U << PARSER_FORMAT_SIZE) - 1U), ANDI >> PARSER_FORMAT_SIZE},
    {"ori",  ORI,  ORI  & ((1U << PARSER_FORMAT_SIZE) - 1U), ORI  >> PARSER_FORMAT_SIZE},
    {"xori", XORI, XORI & ((1U << PARSER_FORMAT_SIZE) - 1U), XORI >> PARSER_FORMAT_SIZE},
    {"slli", SLLI, SLLI & ((1U << PARSER_FORMAT_SIZE) - 1U), SLLI >> PARSER_FORMAT_SIZE},
    {"srli", SRLI, SRLI & ((1U << PARSER_FORMAT_SIZE) - 1U), SRLI >> PARSER_FORMAT_SIZE},
    {"srai", SRAI, SRAI & ((1U << PARSER_FORMAT_SIZE) - 1U), SRAI >> PARSER_FORMAT_SIZE},
    {"ld",   LD,   LD   & ((1U << PARSER_FORMAT_SIZE) - 1U), LD   >> PARSER_FORMAT_SIZE},
    {"st",   ST,   ST   & ((1U << PARSER_FORMAT_SIZE) - 1U), ST   >> PARSER_FORMAT_SIZE},
    {"jlz",  JLZ,  JLZ  & ((1U << PARSER_FORMAT_SIZE) - 1U), JLZ  >> PARSER_FORMAT_SIZE},
    {"jlo",  JLO,  JLO  & ((1U << PARSER_FORMAT_SIZE) - 1U), JLO  >> PARSER_FORMAT_SIZE},
    
    // Fourth row (DSS-Type)
    {"add",  ADD,  ADD  & ((1U << PARSER_FORMAT_SIZE) - 1U), ADD  >> PARSER_FORMAT_SIZE},
    {"sub",  SUB,  SUB  & ((1U << PARSER_FORMAT_SIZE) - 1U), SUB  >> PARSER_FORMAT_SIZE},
    {"and",  AND,  AND  & ((1U << PARSER_FORMAT_SIZE) - 1U), AND  >> PARSER_FORMAT_SIZE},
    {"or",   OR,   OR   & ((1U << PARSER_FORMAT_SIZE) - 1U), OR   >> PARSER_FORMAT_SIZE},
    {"xor",  XOR,  XOR  & ((1U << PARSER_FORMAT_SIZE) - 1U), XOR  >> PARSER_FORMAT_SIZE},
    {"sll",  SLL,  SLL  & ((1U << PARSER_FORMAT_SIZE) - 1U), SLL  >> PARSER_FORMAT_SIZE},
    {"srl",  SRL,  SRL  & ((1U << PARSER_FORMAT_SIZE) - 1U), SRL  >> PARSER_FORMAT_SIZE},
    {"sra",  SRA,  SRA  & ((1U << PARSER_FORMAT_SIZE) - 1U), SRA  >> PARSER_FORMAT_SIZE},
    {"eq",   EQ,   EQ   & ((1U << PARSER_FORMAT_SIZE) - 1U), EQ   >> PARSER_FORMAT_SIZE},
    {"gt",   GT,   GT   & ((1U << PARSER_FORMAT_SIZE) - 1U), GT   >> PARSER_FORMAT_SIZE},
    {"lt",   LT,   LT   & ((1U << PARSER_FORMAT_SIZE) - 1U), LT   >> PARSER_FORMAT_SIZE},
    {"ne",   NE,   NE   & ((1U << PARSER_FORMAT_SIZE) - 1U), NE   >> PARSER_FORMAT_SIZE},
    {"jlrz", JLRZ, JLRZ & ((1U << PARSER_FORMAT_SIZE) - 1U), JLRZ >> PARSER_FORMAT_SIZE},
    {"jlro", JLRO, JLRO & ((1U << PARSER_FORMAT_SIZE) - 1U), JLRO >> PARSER_FORMAT_SIZE},
    
    // Pseudo instructions
    {"j",    JLZ,  PARSER_OPCODE_FORMAT_PSEUDO, PARSER_OPCODE_FORMAT_PSEUDO},
    {"jl",   JLZ,  PARSER_OPCODE_FORMAT_PSEUDO, PARSER_OPCODE_FORMAT_PSEUDO},
    {"jlr",  JLRZ, PARSER_OPCODE_FORMAT_PSEUDO, PARSER_OPCODE_FORMAT_PSEUDO},
    {"jo",   JLO,  PARSER_OPCODE_FORMAT_PSEUDO, PARSER_OPCODE_FORMAT_PSEUDO},
    {"jz",   JLZ,  PARSER_OPCODE_FORMAT_PSEUDO, PARSER_OPCODE_FORMAT_PSEUDO},
    {"call", JLZ,  PARSER_OPCODE_FORMAT_PSEUDO, PARSER_OPCODE_FORMAT_PSEUDO},
    {"li",   ORI,  PARSER_OPCODE_FORMAT_PSEUDO, PARSER_OPCODE_FORMAT_PSEUDO},
    {"mv",   OR,   PARSER_OPCODE_FORMAT_PSEUDO, PARSER_OPCODE_FORMAT_PSEUDO},
    {"nop",  OR,   PARSER_OPCODE_FORMAT_PSEUDO, PARSER_OPCODE_FORMAT_PSEUDO},
    {"ret",  JLRZ, PARSER_OPCODE_FORMAT_PSEUDO, PARSER_OPCODE_FORMAT_PSEUDO}
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


const struct parser_opcode_name *parser_opcode_value_to_name(enum parser_opcode value) {
    size_t n = sizeof(parser_opcode_table) / sizeof(parser_opcode_table[0]);

    for (size_t i = 0; i < n; i++) {
        if (value == parser_opcode_table[i].opcode) {
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


static enum parser_status parser_expect_di_instruction(FILE *file,
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


static enum parser_status parser_expect_ds_instruction(FILE *file,
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


static enum parser_status parser_expect_pseudo_instruction(FILE *file,
                                                           struct parser_group *group,
                                                           struct lexer_token *token)
{
    enum parser_status parse_status;

    if (strncmp(token->text, "j", PARSER_OPCODE_NAME_MAX_LENGTH) == 0) {
        parse_status = parser_expect_i_instruction(file, group, token);
        group->rd = ZERO;
        group->rs1 = ZERO;
    }
    else if (strncmp(token->text, "jl", PARSER_OPCODE_NAME_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(file, group, token);
        group->rs1 = ZERO;
    }
    else if (strncmp(token->text, "jlr", PARSER_OPCODE_NAME_MAX_LENGTH) == 0) {
        parse_status = parser_expect_ds_instruction(file, group, token);
        group->rs2 = group->rs1;
        group->rs1 = ZERO;
    }
    else if (strncmp(token->text, "jo", PARSER_OPCODE_NAME_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(file, group, token);
        group->rs1 = group->rd;
        group->rd = ZERO;
    }
    else if (strncmp(token->text, "jz", PARSER_OPCODE_NAME_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(file, group, token);
        group->rs1 = group->rd;
        group->rd = ZERO;
    }
    else if (strncmp(token->text, "call", PARSER_OPCODE_NAME_MAX_LENGTH) == 0) {
        parse_status = parser_expect_i_instruction(file, group, token);
        group->rd = RA;
        group->rs1 = ZERO;
    }
    else if (strncmp(token->text, "li", PARSER_OPCODE_NAME_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(file, group, token);
        group->rs1 = ZERO;
    }
    else if (strncmp(token->text, "mv", PARSER_OPCODE_NAME_MAX_LENGTH) == 0) {
        parse_status = parser_expect_ds_instruction(file, group, token);
        group->rs2 = ZERO;
    }
    else if (strncmp(token->text, "nop", PARSER_OPCODE_NAME_MAX_LENGTH) == 0) {
        parse_status = PARSER_STATUS_SUCCESS;
        group->rd = ZERO;
        group->rs1 = ZERO;
        group->rs2 = ZERO;
    }
    else if (strncmp(token->text, "ret", PARSER_OPCODE_NAME_MAX_LENGTH) == 0) {
        parse_status = PARSER_STATUS_SUCCESS;
        group->rd = ZERO;
        group->rs1 = ZERO;
        group->rs2 = RA;
    }
    else {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    return parse_status;
}


static enum parser_status parser_expect_instruction(FILE *file,
                                                    struct parser_group *group,
                                                    struct lexer_token *token)
{
    const struct parser_opcode_name *opcode_name = parser_opcode_name_to_value(token->text);

    // TODO: handle pseudo instructions

    group->opcode = opcode_name->opcode;

    if (opcode_name->format == PARSER_OPCODE_FORMAT_PSEUDO) {
        return parser_expect_pseudo_instruction(file, group, token);
    }
    else if (opcode_name->format == PARSER_OPCODE_FORMAT_I) {
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

    group->imm_num = parser_instruction_count;
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

    group->type = PARSER_GROUP_EOF;
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
        group->type = PARSER_GROUP_INSTRUCTION;
        parser_instruction_count += 4;
        return parser_expect_instruction(file, group, token);
    }
    else {
        group->type = PARSER_GROUP_LABEL;
        return parser_expect_label(file, group, token);
    }
}


struct parser_group_node *parser_parse_file(FILE *file, uint16_t base_address) {
    if (file == NULL) {
        return NULL;
    }

    parser_instruction_count = base_address;

    struct parser_group_node *head = NULL;
    struct parser_group_node *curr = NULL;

    struct lexer_token last_token = {0};
    while (true) {
        struct parser_group group = {0};
        enum parser_status parse_status = parser_next_group(file, &group, &last_token);
        switch (parse_status) {
        case PARSER_STATUS_SUCCESS:
            struct parser_group_node *new_node =
                (struct parser_group_node *) malloc(sizeof(struct parser_group_node));
            new_node->group = group;
            new_node->next = NULL;

            if (curr == NULL) {
                head = new_node;
                curr = new_node;
            }
            else {
                curr->next = new_node;
                curr = new_node;
            }
            break;
        case PARSER_STATUS_EOF:
            return head;
        default:
            log_error("unexpected token at line %" PRIu32 ", col %" PRIu32,
                      last_token.line, last_token.column);
            parser_free_group_nodes(head);
            return NULL;
        }
    }
}


void parser_free_group_nodes(struct parser_group_node *head) {
    if (head == NULL) {
        return;
    }

    struct parser_group_node *curr = head;
    while (curr != NULL) {
        struct parser_group_node *next = curr->next;
        free(curr);
        curr = next;
    }
}
