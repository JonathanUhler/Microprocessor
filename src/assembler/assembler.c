#include "assembler/lexer.h"
#include <stdio.h>


int main(void) {
    FILE *file = fopen("program.S", "r");

    struct lexer_token token = {0};
    do {
        enum lexer_status status = lexer_next_token(file, &token);
        if (status != LEXER_STATUS_SUCCESS && status != LEXER_STATUS_EOF) {
            printf("ERROR: Line %d column %d (errno %d)\n", token.line, token.column, status);
            fclose(file);
            return 0;
        }

        printf("Type:  %c\n", token.type);
        printf("Text:  %s\n", token.text);
        printf("Value: %d\n", token.value);
        printf("Loc:   %d:%d\n", token.line, token.column);
        printf("\n");
    } while(token.type != LEXER_TOKEN_EOF);

    fclose(file);
    return 0;
}
