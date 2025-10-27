/**
 * The lexer for the assembler.
 *
 * @author Jonathan Uhler
 */


#ifndef _ASSEMBLER_LEXER_H_
#define _ASSEMBLER_LEXER_H_


#include <stdint.h>
#include <stdio.h>
#include <string.h>


/** The maximum length (EXCLUDING the null terminator) for lexer tokens. */
#define LEXER_TOKEN_MAX_LENGTH 31


/**
 * List of tokens recognized by the lexer.
 */
enum lexer_token_type {
    /** End of the file being parsed/token stream. */
    LEXER_TOKEN_EOF        = '\0',
    /** An identifier (label, directive, or opcode). */
    LEXER_TOKEN_IDENTIFIER = 'i',
    /** The symbolic name of a register (ABI or raw). */
    LEXER_TOKEN_REGISTER   = 'r',
    /** A constant number (decmial or hex). */
    LEXER_TOKEN_NUMBER     = 'x',
    /** A comma separating operands. */
    LEXER_TOKEN_COMMA      = ',',
    /** A colon at the end of a label declaration. */
    LEXER_TOKEN_COLON      = ':',
    /** A period at the beginning of a directive. */
    LEXER_TOKEN_PERIOD     = '.'
};


/**
 * A single token produced by the lexer.
 */
struct lexer_token {
    /** The type of the token. */
    enum lexer_token_type type;
    /** The text of the token, when available. This is intended to be used for identifiers. */
    char text[LEXER_TOKEN_MAX_LENGTH + 1];
    /** The value of the token, when available. This is intended to be used for registers/consts. */
    uint32_t value;
    /** The line number that the token appears on in the source file. */
    uint32_t line;
    /** The column that the token begins at in the source file. */
    uint32_t column;
};


/**
 * The status of the lexer's attempt to fetch a token.
 */
enum lexer_status {
    /** A new token was successfully fetched, the lexer can be called again. */
    LEXER_STATUS_SUCCESS = 0,
    /** No more tokens are available, the lexer should not be called again. */
    LEXER_STATUS_EOF     = 1,
    /** The lexer was called with incorrect arguments. */
    LEXER_STATUS_INVALID_ARGUMENT,
    /** The lexer encountered an unknown character or token while reading the source file. */
    LEXER_STATUS_LEXICAL_ERROR
};


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
enum lexer_status lexer_next_token(FILE *file, struct lexer_token *token);


#endif  // _ASSEMBLER_LEXER_H_
