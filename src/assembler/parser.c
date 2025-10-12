#include "assembler/parser.h"
#include "assembler/lexer.h"


static const struct parser_opcode_name parser_opcode_table[] = {
    // First row (I-Type)
    {"halt", HALT},
    
    // Third row (DSI-Type)
    {"addi", ADDI},
    {"subi", SUBI},
    {"andi", ANDI},
    {"ori", ORI},
    {"xori", XORI},
    {"slli", SLLI},
    {"srli", SRLI},
    {"srai", SRAI},
    {"ld", LD},
    {"st", ST},
    {"jlz", JLZ},
    {"jlo", JLO},
    
    // Fourth row (DSS-Type)
    {"add", ADD},
    {"sub", SUB},
    {"and", AND},
    {"or", OR},
    {"xor", XOR},
    {"sll", SLL},
    {"srl", SRL},
    {"sra", SRA},
    {"eq", EQ},
    {"gt", GT},
    {"lt", LT},
    {"ne", NE},
    {"jlrz", JLRZ},
    {"jlro", JLRO},

    // Pseudo instructions
    {"j", JLZ},
    {"jl", JLZ},
    {"jlr", JLRZ},
    {"jo", JLO},
    {"jz", JLZ},
    {"call", JLZ},
    {"li", ORI},
    {"mv", OR},
    {"nop", OR},
    {"ret", JLRZ}
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


static enum parser_status parser_expect_instruction(FILE *file,
                                                    struct parser_group *group,
                                                    struct lexer_token *first_token)
{
    (void) file;
    (void) group;
    (void) first_token;
    return PARSER_STATUS_SUCCESS;
}


static enum parser_status parser_expect_label(FILE *file,
                                              struct parser_group *group,
                                              struct lexer_token *first_token)
{
    struct lexer_token expected_colon = {0};
    enum lexer_status lex_status = lexer_next_token(file, &expected_colon);
    if (lex_status != LEXER_STATUS_SUCCESS || expected_colon.type != LEXER_TOKEN_COLON) {
        return PARSER_STATUS_SEMANTIC_ERROR;
    }

    strncpy(group->imm_label, first_token->text, LEXER_TOKEN_MAX_LENGTH);
    group->imm_label[LEXER_TOKEN_MAX_LENGTH] = '\0';
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
