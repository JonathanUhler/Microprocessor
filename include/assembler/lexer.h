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
/** The maximum length (EXCLUDING the null terminator) for regsiter ABI names. */
#define LEXER_REGISTER_NAME_MAX_LENGTH 7


/**
 * An entry in a translation table to convert between register ABI/raw names and the binary
 * value to use in instructions.
 */
struct lexer_register_name {
    /** The symbolic name (ABI or raw) of the register. */
    const char *name;
    /** The value of the register to use when (un)packing bits for an instruction. */
    uint32_t value;
};


enum lexer_register {
    ZERO = 0,  R0  = 0,
    RA   = 1,  R1  = 1,
    SP   = 2,  R2  = 2,
    A0   = 3,  R3  = 3,
    A1   = 4,  R4  = 4,
    A2   = 5,  R5  = 5,
    A3   = 6,  R6  = 6,
    A4   = 7,  R7  = 7,
    A5   = 8,  R8  = 8,
    A6   = 9,  R9  = 9,
    A7   = 10, R10 = 10,
    T0   = 11, R11 = 11,
    T1   = 12, R12 = 12,
    T2   = 13, R13 = 13,
    T3   = 14, R14 = 14,
    T4   = 15, R15 = 15,
    T5   = 16, R16 = 16,
    T6   = 17, R17 = 17,
    T7   = 18, R18 = 18,
    S0   = 19, R19 = 19,
    S1   = 20, R20 = 20,
    S2   = 21, R21 = 21,
    S3   = 22, R22 = 22,
    S4   = 23, R23 = 23,
    S5   = 24, R24 = 24,
    S6   = 25, R25 = 25,
    S7   = 26, R26 = 26,
    S8   = 27, R27 = 27,
    S9   = 28, R28 = 28,
    S10  = 29, R29 = 29,
    S11  = 30, R30 = 30,
    S12  = 31, R31 = 31
};


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
    LEXER_TOKEN_COLON      = ':'
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
 * Gets information about the name of a register from a symbolic (ABI or raw) name.
 *
 * @param name  The symbolic (ABI or raw) name of a register, case sensitive.
 *
 * @return A structure containing name information for the requested register, or NULL if no such
 *         register exists.
 */
const struct lexer_register_name *lexer_register_name_to_value(const char *name);


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
