/**
 * The lexer for the assembler.
 *
 * @author Jonathan Uhler
 */


#ifndef _ASSEMBLER_LEXER_H_
#define _ASSEMBLER_LEXER_H_


#include "structures/list.h"
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
    /** A period at the start of directives (e.g. .loc .half). */
    LEXER_TOKEN_PERIOD     = '.',
    /** A double-quoted string token (contents inside quotes). */
    LEXER_TOKEN_STRING     = '\"'
};


/**
 * A single lexed token.
 */
struct lexer_token {
    /** The type of the token. */
    enum lexer_token_type type;
    /** The textual content of the token (null-terminated). */
    char text[LEXER_TOKEN_MAX_LENGTH + 1];
    /** A numeric value (for number/register tokens). */
    uint32_t value;
    /** The name of the source file in which the token appears. */
    const char *file;
    /** The line number that the token appears on in the source file. */
    uint32_t line;
    /** The column that the token begins at in the source file. */
    uint32_t column;
};


/**
 * The status of lexer API functions.
 */
enum lexer_status {
    /** The lexer operation was successful. */
    LEXER_STATUS_SUCCESS = 0,
    /** No more tokens are available, the lexer should not be called again. */
    LEXER_STATUS_EOF,
    /** The lexer was called with incorrect arguments. */
    LEXER_STATUS_INVALID_ARGUMENT,
    /** The lexer encountered an unknown character or token while reading the source file. */
    LEXER_STATUS_LEXICAL_ERROR
};


/**
 * Runs the lexer on the provided input file to read all tokens into a list in they order they
 * appear in the file.
 *
 * Lexing will proceed until the entire file is read (EOF) or an error is encountered. The lexer
 * does not need to be called as a generator to get further tokens after a successful call.
 *
 * @param[inout] file_name  The path to the file to read tokens from.
 * @param[out]   tokens  A pointer to return a list of processed tokens. It is the caller's
 *                       responsibility to free this list with the structures/list API. The tokens
 *                       list must be freed with the list_default_node_free_callback to free the
 *                       node data as well.
 *
 * @return The status of the lexer call. If SUCCESS, the lexer processed all tokens in the file
 *         and stored them in the list output pointer (in a new list allocated by the lexer). If
 *         failure, the lexer encountered an error while parsing the file and should not be called
 *         again. If a non-success status is returned, the caller does not need to free the tokens
 *         list.
 */
enum lexer_status lexer_lex_file(const char *file_name, struct list **tokens);


#endif  // _ASSEMBLER_LEXER_H_
