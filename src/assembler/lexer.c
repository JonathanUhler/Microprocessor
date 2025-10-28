/**
 * The implementation of the assembler lexer.
 *
 * @author Jonathan Uhler
 */


#include "assembler/lexer.h"
#include "architecture/isa.h"
#include "architecture/logger.h"
#include "structures/list.h"
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/** The current line number of the lexer across all calls for the same file. */
static uint32_t lexer_current_line = 1;
/** The current column number of the lexer across all calls for the same file. */
static uint32_t lexer_current_column = 0;


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
 * Checks whether the read pointer in the specified file is currently pointing at the start of
 * a comment, and skips that comment if available.
 *
 * @param[inout] file  The file to skip comments in.
 *
 * @return Whether a comment was successfully skipped.
 */
static bool lexer_skip_comments(FILE *file) {
    int c = fgetc(file);
    if (c != ';') {
        ungetc(c, file);
        return false;
    }

    while (c != '\n') {
        c = fgetc(file);
        lexer_current_column++;
    }
    return true;
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
    case LEXER_TOKEN_COMMA:
    case LEXER_TOKEN_COLON:
    case LEXER_TOKEN_PERIOD:
        token->type = c;
        token->text[0] = c;
        token->text[1] = '\n';
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
    const struct isa_register_map *register_name = isa_get_register_map_from_symbol(token->text);
    if (register_name != NULL) {
        token->type = LEXER_TOKEN_REGISTER;
        token->value = register_name->index;
    }
    else {
        token->type = LEXER_TOKEN_IDENTIFIER;
    }
    return true;
}


/**
 * Runs the lexer on the provided input file to get the next token.
 *
 * Lexing will continue at wherever the read pointer is in the file. This function will advance
 * the read pointer of the file while processing characters; if the lexer is called again, the
 * file pointer should not be changed between calls.
 *
 * @param[inout] file   The file to read a token from.
 * @param[out]   token  A pointer to store the token information.
 *
 * @return The status of the lexer call. If SUCCESS, the lexer can be called again to get another
 *         token. If EOF, the lexer exited normally but should not be called again. Otherwise, the
 *         lexer exited with an error (the line, column, and offending character are stored in the
 *         token pointer).
 */
static enum lexer_status lexer_next_token(FILE *file, struct lexer_token *token) {
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
    log_trace("Lexer checking for whitespace to skip (%" PRIu32 ":%" PRIu32 ")",
              lexer_current_line, lexer_current_column);
    if (!lexer_skip_whitespace(file)) {
        lexer_current_line = 1;
        lexer_current_column = 0;
        return LEXER_STATUS_EOF;
    }

    // Skip comments from the current file pointer. If this returns true, that means a comment
    // was skipped (from the ; symbol to end of line) and so we want to continue to get the next
    // real token on the start of the next line.
    log_trace("Lexer checking for comments to skip (%" PRIu32 ":%" PRIu32 ")",
              lexer_current_line, lexer_current_column);
    if (lexer_skip_comments(file)) {
        lexer_current_line++;
        lexer_current_column = 0;
        return lexer_next_token(file, token);
    }

    // At this point we have reached a real (non-whitespace, non-comment) character to parse.
    // We go through all the token parsers in order trying to find a lexical match.

    log_trace("Lexer checking for punctuation (%" PRIu32 ":%" PRIu32 ")",
              lexer_current_line, lexer_current_column);
    token->line = lexer_current_line;
    token->column = lexer_current_column;
    if (lexer_check_punctuation(file, token)) {
        return LEXER_STATUS_SUCCESS;
    }

    log_trace("Lexer checking for number (%" PRIu32 ":%" PRIu32 ")",
              lexer_current_line, lexer_current_column);
    token->line = lexer_current_line;
    token->column = lexer_current_column;
    if (lexer_check_number(file, token)) {
        return LEXER_STATUS_SUCCESS;
    }

    log_trace("Lexer checking for identifier (%" PRIu32 ":%" PRIu32 ")",
              lexer_current_line, lexer_current_column);
    token->line = lexer_current_line;
    token->column = lexer_current_column;
    if (lexer_check_identifier(file, token)) {
        return LEXER_STATUS_SUCCESS;
    }

    // At this point all of the parsers have failed. That means we have an unrecognized token
    // and parsing the file cannot continue.
    log_trace("Lexer did not identify any known token (%" PRIu32 ":%" PRIu32 ")",
              lexer_current_line, lexer_current_column);
    token->text[0] = fgetc(file);
    token->text[1] = '\0';
    return LEXER_STATUS_LEXICAL_ERROR;
}


enum lexer_status lexer_lex_file(FILE *file, struct list **tokens) {
    if (file == NULL || tokens == NULL) {
        return LEXER_STATUS_INVALID_ARGUMENT;
    }

    *tokens = create_list();

    while (true) {
        struct lexer_token *token = (struct lexer_token *) calloc(1, sizeof(struct lexer_token));
        enum lexer_status lex_status = lexer_next_token(file, token);

        switch (lex_status) {
        case LEXER_STATUS_SUCCESS:
            log_debug("Lexer found token of type '%c'", token->type);
            list_add(*tokens, (void *) token);
            break;
        case LEXER_STATUS_EOF:
            log_info("Lexer finished successfully (tokens found: %" PRIu32 ")", (*tokens)->size);
            free(token);
            return LEXER_STATUS_SUCCESS;
        default:
            log_error("Lexer could not parse token at line %" PRIu32 ", col %" PRIu32 " (errno %d)",
                      lexer_current_line, lexer_current_column, lex_status);
            destroy_list(*tokens, &list_default_node_free_callback);
             free(token);
            return lex_status;
        }
    }
}
