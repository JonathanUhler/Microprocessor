#include "assembler/parser.h"
#include "assembler/lexer.h"
#include "architecture/isa.h"
#include "architecture/logger.h"
#include "structures/list.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


static uint32_t parser_pc = 0;
static struct lexer_token *parser_last_token = NULL;


static enum parser_status parser_expect_sequence(struct list *tokens,
                                                 struct list *sequence) {
    for (uint32_t i = 0; i < sequence->size; i++) {
        void *token_data;
        void *type_data;
        enum list_status list_status;

        list_status = list_peek_at(tokens, i, &token_data);
        if (list_status != LIST_STATUS_SUCCESS) {
            return PARSER_STATUS_SEMANTIC_ERROR;
        }

        list_status = list_peek_at(sequence, i, &type_data);
        if (list_status != LIST_STATUS_SUCCESS) {
            return PARSER_STATUS_SEMANTIC_ERROR;
        }

        struct lexer_token *token = (struct lexer_token *) token_data;
        enum lexer_token_type *type = (enum lexer_token_type *) type_data;
        log_trace("Parser checking sequence[%" PRIu32 "] = '%c' vs '%c'", i, *type, token->type);
        parser_last_token = token;
        if (token->type != *type) {
            return PARSER_STATUS_SEMANTIC_ERROR;
        }
    }

    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_blank_instruction(struct list *tokens)
{
    log_debug("Parser checking for blank instruction");

    enum lexer_token_type identifier = LEXER_TOKEN_IDENTIFIER;

    struct list *sequence = create_list();
    list_add(sequence, &identifier);

    enum parser_status match_status = parser_expect_sequence(tokens, sequence);
    destroy_list(sequence, NULL);
    if (match_status != PARSER_STATUS_SUCCESS) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    void *data;
    struct lexer_token *token;

    // Identifier
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);

    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_i_instruction(struct list *tokens,
                                                      struct parser_group *group)
{
    log_debug("Parser checking for I-type instruction");

    enum lexer_token_type identifier = LEXER_TOKEN_IDENTIFIER;
    enum lexer_token_type number = LEXER_TOKEN_NUMBER;

    struct list *label_sequence = create_list();
    list_add(label_sequence, &identifier);
    list_add(label_sequence, &identifier);
    struct list *const_sequence = create_list();
    list_add(const_sequence, &identifier);
    list_add(const_sequence, &number);

    enum parser_status label_match_status = parser_expect_sequence(tokens, label_sequence);
    enum parser_status const_match_status = parser_expect_sequence(tokens, const_sequence);
    destroy_list(label_sequence, NULL);
    destroy_list(const_sequence, NULL);
    if (label_match_status != PARSER_STATUS_SUCCESS && const_match_status != PARSER_STATUS_SUCCESS)
    {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    void *data;
    struct lexer_token *token;

    // Identifier
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);
    // Identifier or number
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    if (label_match_status == PARSER_STATUS_SUCCESS) {
        memcpy(group->instruction.label, token->text, LEXER_TOKEN_MAX_LENGTH);
        group->instruction.label[LEXER_TOKEN_MAX_LENGTH] = '\0';
    }
    else {
        group->instruction.immediate = token->value;
    }
    free(token);

    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_di_instruction(struct list *tokens,
                                                       struct parser_group *group)
{
    log_debug("Parser checking for DI-type pseudo-instruction");

    enum lexer_token_type identifier = LEXER_TOKEN_IDENTIFIER;
    enum lexer_token_type destination = LEXER_TOKEN_REGISTER;
    enum lexer_token_type number = LEXER_TOKEN_NUMBER;
    enum lexer_token_type comma = LEXER_TOKEN_COMMA;

    struct list *label_sequence = create_list();
    list_add(label_sequence, &identifier);
    list_add(label_sequence, &destination);
    list_add(label_sequence, &comma);
    list_add(label_sequence, &identifier);
    struct list *const_sequence = create_list();
    list_add(const_sequence, &identifier);
    list_add(const_sequence, &destination);
    list_add(const_sequence, &comma);
    list_add(const_sequence, &number);

    enum parser_status label_match_status = parser_expect_sequence(tokens, label_sequence);
    enum parser_status const_match_status = parser_expect_sequence(tokens, const_sequence);
    destroy_list(label_sequence, NULL);
    destroy_list(const_sequence, NULL);
    if (label_match_status != PARSER_STATUS_SUCCESS && const_match_status != PARSER_STATUS_SUCCESS)
    {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    void *data;
    struct lexer_token *token;

    // Identifier
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);
    // Destination
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    group->instruction.dest = token->value;
    free(token);
    // Comma
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);
    // Identifier or number
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    if (label_match_status == PARSER_STATUS_SUCCESS) {
        memcpy(group->instruction.label, token->text, LEXER_TOKEN_MAX_LENGTH);
        group->instruction.label[LEXER_TOKEN_MAX_LENGTH] = '\0';
    }
    else {
        group->instruction.immediate = token->value;
    }
    free(token);

    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_ds_instruction(struct list *tokens,
                                                       struct parser_group *group)
{
    log_debug("Parser checking for DS-type pseudo-instruction");

    enum lexer_token_type identifier = LEXER_TOKEN_IDENTIFIER;
    enum lexer_token_type destination = LEXER_TOKEN_REGISTER;
    enum lexer_token_type source = LEXER_TOKEN_REGISTER;
    enum lexer_token_type comma = LEXER_TOKEN_COMMA;

    struct list *sequence = create_list();
    list_add(sequence, &identifier);
    list_add(sequence, &destination);
    list_add(sequence, &comma);
    list_add(sequence, &source);

    enum parser_status match_status = parser_expect_sequence(tokens, sequence);
    destroy_list(sequence, NULL);
    if (match_status != PARSER_STATUS_SUCCESS) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    void *data;
    struct lexer_token *token;

    // Identifier
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);
    // Destination
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    group->instruction.dest = token->value;
    free(token);
    // Comma
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);
    // Source
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    group->instruction.source1 = token->value;
    free(token);

    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_dsi_instruction(struct list *tokens,
                                                        struct parser_group *group)
{
    log_debug("Parser checking for DSI-type instruction");

    enum lexer_token_type identifier = LEXER_TOKEN_IDENTIFIER;
    enum lexer_token_type destination = LEXER_TOKEN_REGISTER;
    enum lexer_token_type source = LEXER_TOKEN_REGISTER;
    enum lexer_token_type number = LEXER_TOKEN_NUMBER;
    enum lexer_token_type comma = LEXER_TOKEN_COMMA;

    struct list *label_sequence = create_list();
    list_add(label_sequence, &identifier);
    list_add(label_sequence, &destination);
    list_add(label_sequence, &comma);
    list_add(label_sequence, &source);
    list_add(label_sequence, &comma);
    list_add(label_sequence, &identifier);
    struct list *const_sequence = create_list();
    list_add(label_sequence, &identifier);
    list_add(label_sequence, &destination);
    list_add(label_sequence, &comma);
    list_add(label_sequence, &source);
    list_add(label_sequence, &comma);
    list_add(label_sequence, &number);

    enum parser_status label_match_status = parser_expect_sequence(tokens, label_sequence);
    enum parser_status const_match_status = parser_expect_sequence(tokens, const_sequence);
    destroy_list(label_sequence, NULL);
    destroy_list(const_sequence, NULL);
    if (label_match_status != PARSER_STATUS_SUCCESS && const_match_status != PARSER_STATUS_SUCCESS)
    {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    void *data;
    struct lexer_token *token;

    // Identifier
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);
    // Destination
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    group->instruction.dest = token->value;
    free(token);
    // Comma
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);
    // Destination
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    group->instruction.source1 = token->value;
    free(token);
    // Comma
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);
    // Identifier or number
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    if (label_match_status == PARSER_STATUS_SUCCESS) {
        memcpy(group->instruction.label, token->text, LEXER_TOKEN_MAX_LENGTH);
        group->instruction.label[LEXER_TOKEN_MAX_LENGTH] = '\0';
    }
    else {
        group->instruction.immediate = token->value;
    }
    free(token);

    return PARSER_STATUS_SUCCESS;

}


static enum parser_status parser_expect_dss_instruction(struct list *tokens,
                                                        struct parser_group *group)
{
    log_debug("Parser checking for DSS-type instruction");

    enum lexer_token_type identifier = LEXER_TOKEN_IDENTIFIER;
    enum lexer_token_type destination = LEXER_TOKEN_REGISTER;
    enum lexer_token_type source = LEXER_TOKEN_REGISTER;
    enum lexer_token_type comma = LEXER_TOKEN_COMMA;

    struct list *sequence = create_list();
    list_add(sequence, &identifier);
    list_add(sequence, &destination);
    list_add(sequence, &comma);
    list_add(sequence, &source);
    list_add(sequence, &comma);
    list_add(sequence, &source);

    enum parser_status match_status = parser_expect_sequence(tokens, sequence);
    destroy_list(sequence, NULL);
    if (match_status != PARSER_STATUS_SUCCESS) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    void *data;
    struct lexer_token *token;

    // Identifier
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);
    // Destination
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    group->instruction.dest = token->value;
    free(token);
    // Comma
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);
    // Source
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    group->instruction.source1 = token->value;
    free(token);
    // Comma
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);
    // Source
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    group->instruction.source2 = token->value;
    free(token);

    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_pseudo_instruction(struct list *tokens,
                                                           struct parser_group *group)
{
    log_debug("Parser checking for pseudo-instruction");

    void *data;
    list_peek_at(tokens, 0, &data);
    struct lexer_token *token = (struct lexer_token *) data;

    enum parser_status parse_status;
    if (strncmp(token->text, "j", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_i_instruction(tokens, group);
        group->instruction.dest = ZERO;
        group->instruction.source1 = ZERO;
    }
    else if (strncmp(token->text, "jl", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(tokens, group);
        group->instruction.source1 = ZERO;
    }
    else if (strncmp(token->text, "jlr", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_ds_instruction(tokens, group);
        group->instruction.source2 = group->instruction.source1;
        group->instruction.source1 = ZERO;
    }
    else if (strncmp(token->text, "j1", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(tokens, group);
        group->instruction.source1 = group->instruction.dest;
        group->instruction.dest = ZERO;
    }
    else if (strncmp(token->text, "j0", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(tokens, group);
        group->instruction.source1 = group->instruction.dest;
        group->instruction.dest = ZERO;
    }
    else if (strncmp(token->text, "call", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_i_instruction(tokens, group);
        group->instruction.dest = RA;
        group->instruction.source1 = ZERO;
    }
    else if (strncmp(token->text, "li", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_di_instruction(tokens, group);
        group->instruction.source1 = ZERO;
    }
    else if (strncmp(token->text, "mv", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_ds_instruction(tokens, group);
        group->instruction.source2 = ZERO;
    }
    else if (strncmp(token->text, "nop", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_blank_instruction(tokens);
        group->instruction.dest = ZERO;
        group->instruction.source1 = ZERO;
        group->instruction.source2 = ZERO;
    }
    else if (strncmp(token->text, "ret", ISA_OPCODE_SYMBOL_MAX_LENGTH) == 0) {
        parse_status = parser_expect_blank_instruction(tokens);
        group->instruction.dest = ZERO;
        group->instruction.source1 = ZERO;
        group->instruction.source2 = RA;
    }
    else {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    return parse_status;
}


static enum parser_status parser_expect_instruction(struct list *tokens, struct parser_group *group)
{
    log_debug("Parser checking for instruction");
    group->type = PARSER_GROUP_INSTRUCTION;

    void *data;
    enum list_status list_status = list_peek_at(tokens, 0, &data);
    if (list_status != LIST_STATUS_SUCCESS) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    struct lexer_token *token = (struct lexer_token *) data;
    if (token->type != LEXER_TOKEN_IDENTIFIER) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    const struct isa_opcode_map *opcode_map = isa_get_opcode_map_from_symbol(token->text);
    if (opcode_map == NULL) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    group->instruction.opcode = opcode_map->opcode;
    switch (opcode_map->format) {
    case ISA_OPCODE_FORMAT_PSEUDO:
        return parser_expect_pseudo_instruction(tokens, group);
    case ISA_OPCODE_FORMAT_I:
        return parser_expect_i_instruction(tokens, group);
    case ISA_OPCODE_FORMAT_DSI:
        return parser_expect_dsi_instruction(tokens, group);
    case ISA_OPCODE_FORMAT_DSS:
        return parser_expect_dss_instruction(tokens, group);
    default:
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
}


static enum parser_status parser_expect_label(struct list *tokens, struct parser_group *group) {
    log_debug("Parser checking for label");
    group->type = PARSER_GROUP_LABEL;

    enum lexer_token_type identifier = LEXER_TOKEN_IDENTIFIER;
    enum lexer_token_type colon = LEXER_TOKEN_COLON;
    struct list *sequence = create_list();
    list_add(sequence, &identifier);
    list_add(sequence, &colon);

    enum parser_status match_status = parser_expect_sequence(tokens, sequence);
    destroy_list(sequence, NULL);
    if (match_status != PARSER_STATUS_SUCCESS) {
        return match_status;
    }

    void *data;
    struct lexer_token *token;

    // Identifier
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    memcpy(group->label.label, token->text, LEXER_TOKEN_MAX_LENGTH);
    group->label.label[LEXER_TOKEN_MAX_LENGTH] = '\0';
    group->label.immediate = parser_pc;
    free(token);
    // Colon
    list_pop_front(tokens, &data);
    token = (struct lexer_token *) data;
    free(token);

    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_next_group(struct list *tokens,
                                            struct parser_group *group)
{
    if (tokens->size == 0) {
        return PARSER_STATUS_EOF;
    }

    if (parser_expect_label(tokens, group) == PARSER_STATUS_SUCCESS) {
        log_debug("Parser found a label '%s' at 0x%04" PRIx16, group->label.label, parser_pc);
        return PARSER_STATUS_SUCCESS;
    }
    else if (parser_expect_instruction(tokens, group) == PARSER_STATUS_SUCCESS) {
        log_debug("Parser found an instruction");
        parser_pc += sizeof(uint32_t);
        return PARSER_STATUS_SUCCESS;
    }
    else {
        group->type = PARSER_GROUP_EOF;
        return PARSER_STATUS_SEMANTIC_ERROR;
    }
}


enum parser_status parser_parse_tokens(struct list *tokens,
                                       uint16_t base_address,
                                       struct list **groups)
{
    if (tokens == NULL || groups == NULL) {
        return PARSER_STATUS_INVALID_ARGUMENT;
    }

    parser_pc = base_address;
    *groups = create_list();

    uint32_t num_groups = 0;
    while (true) {
        num_groups++;
        struct parser_group *group = (struct parser_group *) calloc(1, sizeof(struct parser_group));
        enum parser_status parse_status = parser_next_group(tokens, group);

        switch (parse_status) {
        case PARSER_STATUS_SUCCESS:
            log_debug("Parser found semantic group of type %d", group->type);
            list_add(*groups, (void *) group);
            break;
        case PARSER_STATUS_EOF:
            log_info("Parser reached end-of-file successfully");
            free(group);
            return PARSER_STATUS_SUCCESS;
        default:
            log_error("%" PRIu32 ":%" PRIu32 ": Parser could not parse token (errno %d)",
                      parser_last_token != NULL ? parser_last_token->line : 0,
                      parser_last_token != NULL ? parser_last_token->column : 0,
                      parse_status);
            destroy_list(*groups, &list_default_node_free_callback);
            free(group);
            return parse_status;
        }
    }
}
