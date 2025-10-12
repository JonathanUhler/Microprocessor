#include "assembler/lexer.h"
#include "assembler/parser.h"
#include <stdio.h>


int main(void) {
    FILE *file = fopen("program.S", "r");

    enum parser_status status = PARSER_STATUS_EOF;
    struct lexer_token token = {0};
    do {
        struct parser_group group = {0};
        status = parser_next_group(file, &group, &token);
        if (status == PARSER_STATUS_SUCCESS) {
            printf("GOOD GROUP, label is %s\n", group.imm_label);
        }
        else {
            printf("ERROR: parser failed\n");
            switch (token.type) {
            case LEXER_TOKEN_EOF:
                printf("EOF\n");
                break;
            case LEXER_TOKEN_IDENTIFIER:
                printf("IDENT(%s)\n", token.text);
                break;
            case LEXER_TOKEN_REGISTER:
                printf("REGISTER(%s,%d)\n", token.text, token.value);
                break;
            case LEXER_TOKEN_NUMBER:
                printf("NUMBER(%d)\n", token.value);
                break;
            case LEXER_TOKEN_COMMA:
                printf("COMMA\n");
                break;
            case LEXER_TOKEN_COLON:
                printf("COLON\n");
                break;
            }
        }
    } while (status != PARSER_STATUS_EOF);

    /*
    struct lexer_token token = {0};
    do {
        enum lexer_status status = lexer_next_token(file, &token);
        if (status != LEXER_STATUS_SUCCESS && status != LEXER_STATUS_EOF) {
            printf("ERROR: Line %d column %d (errno %d)\n", token.line, token.column, status);
            fclose(file);
            return 0;
        }

        switch (token.type) {
        case LEXER_TOKEN_EOF:
            printf("EOF\n");
            break;
        case LEXER_TOKEN_IDENTIFIER:
            printf("IDENT(%s)\n", token.text);
            break;
        case LEXER_TOKEN_REGISTER:
            printf("REGISTER(%s,%d)\n", token.text, token.value);
            break;
        case LEXER_TOKEN_NUMBER:
            printf("NUMBER(%d)\n", token.value);
            break;
        case LEXER_TOKEN_COMMA:
            printf("COMMA\n");
            break;
        case LEXER_TOKEN_COLON:
            printf("COLON\n");
            break;
        }
    } while(token.type != LEXER_TOKEN_EOF);
    */

    fclose(file);
    return 0;
}
