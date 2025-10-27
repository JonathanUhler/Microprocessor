#include "assembler/parser.h"
#include "assembler/lexer.h"
#include "architecture/isa.h"
#include "architecture/logger.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


static uint32_t parser_pc = 0;


static enum parser_status parser_expect_register(FILE *file, struct lexer_token *token) {
    log_trace("Parser expecting register");
    enum lexer_status lex_status = lexer_next_token(file, token);
    if (lex_status != LEXER_STATUS_SUCCESS || token->type != LEXER_TOKEN_REGISTER) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_immediate(FILE *file, struct lexer_token *token) {
    log_trace("Parser expecting immediate constant or label");
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
    log_trace("Parser expecting comma separator");
    enum lexer_status lex_status = lexer_next_token(file, token);
    if (lex_status != LEXER_STATUS_SUCCESS || token->type != LEXER_TOKEN_COMMA) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_colon(FILE *file, struct lexer_token *token) {
    log_trace("Parser expecting colon separator");
    enum lexer_status lex_status = lexer_next_token(file, token);
    if (lex_status != LEXER_STATUS_SUCCESS || token->type != LEXER_TOKEN_COLON) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_period(FILE *file, struct lexer_token *token) {
    log_trace("Parser expecting period separator");
    enum lexer_status lex_status = lexer_next_token(file, token);
    if (lex_status != LEXER_STATUS_SUCCESS || token->type != LEXER_TOKEN_PERIOD) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_i_instruction(FILE *file,
                                                      struct parser_group *group,
                                                      struct lexer_token *token)
{
    log_trace("Parser expecting I-type instruction");
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
    log_trace("Parser expecting DI-type pseudo-instruction");
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
    log_trace("Parser expecting DS-type pseudo-instruction");
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
    log_trace("Parser expecting DSI-type instruction");
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
    log_trace("Parser expecting DSS-type instruction");
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

    if (strncmp(token->text, "j", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_i_instruction(file, group, token);
        group->rd = ZERO;
        group->rs1 = ZERO;
    }
    else if (strncmp(token->text, "jl", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(file, group, token);
        group->rs1 = ZERO;
    }
    else if (strncmp(token->text, "jlr", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_ds_instruction(file, group, token);
        group->rs2 = group->rs1;
        group->rs1 = ZERO;
    }
    else if (strncmp(token->text, "j1", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(file, group, token);
        group->rs1 = group->rd;
        group->rd = ZERO;
    }
    else if (strncmp(token->text, "j0", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(file, group, token);
        group->rs1 = group->rd;
        group->rd = ZERO;
    }
    else if (strncmp(token->text, "call", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_i_instruction(file, group, token);
        group->rd = RA;
        group->rs1 = ZERO;
    }
    else if (strncmp(token->text, "li", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(file, group, token);
        group->rs1 = ZERO;
    }
    else if (strncmp(token->text, "mv", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_ds_instruction(file, group, token);
        group->rs2 = ZERO;
    }
    else if (strncmp(token->text, "nop", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = PARSER_STATUS_SUCCESS;
        group->rd = ZERO;
        group->rs1 = ZERO;
        group->rs2 = ZERO;
    }
    else if (strncmp(token->text, "ret", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
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
    const struct isa_opcode_map *opcode_map = isa_get_opcode_map_from_symbol(token->text);
    group->opcode = opcode_map->opcode;

    if (opcode_map->format == ISA_OPCODE_FORMAT_PSEUDO) {
        return parser_expect_pseudo_instruction(file, group, token);
    }
    else if (opcode_map->format == ISA_OPCODE_FORMAT_I) {
        return parser_expect_i_instruction(file, group, token);
    }
    else if (opcode_map->format == ISA_OPCODE_FORMAT_DSI) {
        return parser_expect_dsi_instruction(file, group, token);
    }
    else if (opcode_map->format == ISA_OPCODE_FORMAT_DSS) {
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

    group->imm_num = parser_pc;
    memcpy(group->imm_label, token->text, LEXER_TOKEN_MAX_LENGTH);
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

    const struct isa_opcode_map *opcode_map = isa_get_opcode_map_from_symbol(token->text);
    if (opcode_map != NULL) {
        log_debug("Parser found opcode map for instruction '%s'", token->text);
        group->type = PARSER_GROUP_INSTRUCTION;
        parser_pc += sizeof(uint32_t);
        return parser_expect_instruction(file, group, token);
    }
    else {
        log_debug("Parser found no opcode map, expecting label");
        group->type = PARSER_GROUP_LABEL;
        return parser_expect_label(file, group, token);
    }
}


struct parser_group_node *parser_parse_file(FILE *file, uint16_t base_address) {
    if (file == NULL) {
        return NULL;
    }

    parser_pc = base_address;

    struct parser_group_node *head = NULL;
    struct parser_group_node *curr = NULL;

    struct lexer_token last_token = {0};
    uint32_t num_groups = 0;
    while (true) {
        num_groups++;

        struct parser_group group = {0};
        enum parser_status parse_status = parser_next_group(file, &group, &last_token);

        switch (parse_status) {
        case PARSER_STATUS_SUCCESS:
            log_info("Parsed semantic group %" PRIu32 " successfully", num_groups);
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
            log_info("Parser reached end-of-file");
            return head;
        default:
            log_error("Unexpected token at line %" PRIu32 ", col %" PRIu32,
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
