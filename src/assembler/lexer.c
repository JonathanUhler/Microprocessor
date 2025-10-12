/**
 * The implementation of the assembler lexer.
 *
 * @author Jonathan Uhler
 */


#include "assembler/lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/** The current line number of the lexer across all calls for the same file. */
static uint32_t lexer_current_line = 1;
/** The current column number of the lexer across all calls for the same file. */
static uint32_t lexer_current_column = 0;


/** A mapping of symbolic register names (ABI and raw) to their encoded values. */
static const struct lexer_register_name lexer_register_table[] = {
    /*  ABI          Raw  */
    {"zero", 0}, {"r0", 0},
    {"ra", 1},   {"r1", 1},
    {"sp", 2},   {"r2", 2},
    {"a0", 3},   {"r3", 3},
    {"a1", 4},   {"r4", 4},
    {"a2", 5},   {"r5", 5},
    {"a3", 6},   {"r6", 6},
    {"a4", 7},   {"r7", 7},
    {"a5", 8},   {"r8", 8},
    {"a6", 9},   {"r9", 9},
    {"a7", 10},  {"r10", 10},
    {"t0", 11},  {"r11", 11},
    {"t1", 12},  {"r12", 12},
    {"t2", 13},  {"r13", 13},
    {"t3", 14},  {"r14", 14},
    {"t4", 15},  {"r15", 15},
    {"t5", 16},  {"r16", 16},
    {"t6", 17},  {"r17", 17},
    {"t7", 18},  {"r18", 18},
    {"s0", 19},  {"r19", 19},
    {"s1", 20},  {"r20", 20},
    {"s2", 21},  {"r21", 21},
    {"s3", 22},  {"r22", 22},
    {"s4", 23},  {"r23", 23},
    {"s5", 24},  {"r24", 24},
    {"s6", 25},  {"r25", 25},
    {"s7", 26},  {"r26", 26},
    {"s8", 27},  {"r27", 27},
    {"s9", 28},  {"r28", 28},
    {"s10", 29}, {"r29", 29},
    {"s11", 30}, {"r30", 30},
    {"s12", 31}, {"r31", 31}
};


/**
 * Skips all whitespace in the specified file starting at the current read pointer.
 *
 * @param[inout] file  The file to skip whitespace in.
 *
 * @return Whether whitespace was successfully skipped without reaching the end of the file.
 */
static bool lexer_skip_whitespace(FILE *file) {
    int c;
    do {
        c = fgetc(file);
        lexer_current_column++;

        if (c == '\n') {
            lexer_current_line++;
            lexer_current_column = 0;
        }
    } while (isspace(c));

    if (c != EOF) {
        ungetc(c, file);
        lexer_current_column--;
        return true;
    }
    return false;
}


/**
 * Checks whether the read pointer in the specified file is currently pointing at a punctuation
 * token, and parses that token if possible.
 *
 * @param[inout] file   The file to parse from.
 * @param[out]   token  A pointer to store token information.
 *
 * @return Whether a punctuation token was parsed from the file.
 */
static bool lexer_check_punctuation(FILE *file, struct lexer_token *token) {
    int c = fgetc(file);

    switch (c) {
    case ',':
        token->type = LEXER_TOKEN_COMMA;
        strncpy(token->text, ",", LEXER_TOKEN_MAX_LENGTH);
        lexer_current_column++;
        return true;
    case ':':
        token->type = LEXER_TOKEN_COLON;
        strncpy(token->text, ":", LEXER_TOKEN_MAX_LENGTH);
        lexer_current_column++;
        return true;
    default:
        if (c != EOF) {
            ungetc(c, file);
        }
        return false;
    }
}


/**
 * Checks whether the read pointer in the specified file is currently pointing at a number token,
 * and parses that token if possible.
 *
 * @param[inout] file   The file to parse from.
 * @param[out]   token  A pointer to store token information.
 *
 * @return Whether a number token was parsed from the file.
 */
static bool lexer_check_number(FILE *file, struct lexer_token *token) {
    // If the first character we read isn't a digit (any digit for decimal, and '0' for the '0x' in
    // hex specifically), it cannot possibly be a number.
    int c = fgetc(file);
    if (!isdigit(c)) {
        ungetc(c, file);
        return false;
    }

    // Determine the base. If the digit we read above is a '0' and the next is an 'x', we know the
    // number is hexadecimal.
    uint32_t base = 10;
    uint32_t value = 0;

    int peekc = fgetc(file);
    if (c == '0' && tolower(peekc) == 'x') {
        base = 16;
        c = fgetc(file);
        lexer_current_column++;
    }
    else {
        if (peekc != EOF) {
            ungetc(peekc, file);
        }
    }

    // Read the rest of the digits (hex or dec) and construct the number in-place.
    while ((base == 10 && isdigit(c)) || (base == 16 && isxdigit(c))) {
        value = value * base + (isdigit(c) ? c - '0' : tolower(c) - 'a' + 10);
        c = fgetc(file);
        lexer_current_column++;
    }

    if (c != EOF) {
        ungetc(c, file);
    }
    token->type = LEXER_TOKEN_NUMBER;
    token->value = value;
    return true;
}


const struct lexer_register_name *lexer_register_name_to_value(const char *name) {
    size_t n = sizeof(lexer_register_table) / sizeof(lexer_register_table[0]);

    for (size_t i = 0; i < n; i++) {
        if (strncmp(name, lexer_register_table[i].name, LEXER_REGISTER_NAME_MAX_LENGTH) == 0) {
            return &lexer_register_table[i];
        }
    }

    return NULL;
}


/**
 * Checks whether the read pointer in the specified file is currently pointing at an identifier
 * or register token, and parses that token if possible.
 *
 * @param[inout] file   The file to parse from.
 * @param[out]   token  A pointer to store token information.
 *
 * @return Whether an identifier or register token was parsed from the file.
 */
static bool lexer_check_identifier(FILE *file, struct lexer_token *token) {
    // Check for the first character, which is C-style (alpha + _ but no numbers)
    int c = fgetc(file);
    if (!isalpha(c) && c != '_') {
        ungetc(c, file);
        return false;
    }

    // Read additional characters until we find one that doesn't belong in an identifier.
    uint32_t i = 0;
    do {
        token->text[i++] = c;
        c = fgetc(file);
        lexer_current_column++;
    } while ((isalnum(c) || c == '_') && i < LEXER_TOKEN_MAX_LENGTH);

    token->text[i] = '\0';
    if (c != EOF) {
        ungetc(c, file);
    }

    // At this point, we have a full identifier (null terminated) in token->text. Since labels,
    // opcodes, and register names all fit the definition of an "identifier", we need to determine
    // if the identifier is a register and get its encoded value.
    const struct lexer_register_name *register_name = lexer_register_name_to_value(token->text);
    if (register_name != NULL) {
        token->type = LEXER_TOKEN_REGISTER;
        token->value = register_name->value;
    }
    else {
        token->type = LEXER_TOKEN_IDENTIFIER;
    }
    return true;
}


enum lexer_status lexer_next_token(FILE *file, struct lexer_token *token) {
    if (file == NULL || token == NULL) {
        return LEXER_STATUS_INVALID_ARGUMENT;
    }

    // At this point we know that the arguments are at least valid and parsing can be attempted.
    // We clear the token parameters and fill them as we encounter tokens.
    token->type = LEXER_TOKEN_EOF;
    memset(&token->text, 0, sizeof(token->text));
    token->value = 0;
    token->line = 0;
    token->column = 0;

    // Skip whitespace from the current file read pointer. If this returns false, that means the
    // end of the file was reached while skipping whitespace. In that case, we reset the global
    // line/column counters (for the next file) and return EOF.
    if (!lexer_skip_whitespace(file)) {
        lexer_current_line = 1;
        lexer_current_column = 0;
        return LEXER_STATUS_EOF;
    }

    // At this point we have reached a real (non-whitespace, non-comment) character to parse.
    // We go through all the token parsers in order trying to find a lexical match.

    token->line = lexer_current_line;
    token->column = lexer_current_column;
    if (lexer_check_punctuation(file, token)) {
        return LEXER_STATUS_SUCCESS;
    }

    token->line = lexer_current_line;
    token->column = lexer_current_column;
    if (lexer_check_number(file, token)) {
        return LEXER_STATUS_SUCCESS;
    }

    token->line = lexer_current_line;
    token->column = lexer_current_column;
    if (lexer_check_identifier(file, token)) {
        return LEXER_STATUS_SUCCESS;
    }

    // At this point all of the parsers have failed. That means we have an unrecognized token
    // and parsing the file cannot continue.
    token->text[0] = fgetc(file);
    token->text[1] = '\0';
    return LEXER_STATUS_UNKNOWN_TOKEN;
}
