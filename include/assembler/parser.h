/**
 * Parser to convert tokens emitted by the lexer into semantic groups.
 *
 * @author Jonathan Uhler
 */


#ifndef _ASSEMBLER_PARSER_H_
#define _ASSEMBLER_PARSER_H_


#include "assembler/lexer.h"
#include "architecture/isa.h"
#include <stdint.h>
#include <stdio.h>


/**
 * The possible semantic group types.
 */
enum parser_group_type {
    /** A group of tokens that form an instruction (any Format/type, including pseudo). */
    PARSER_GROUP_INSTRUCTION,
    /** A group of tokens that define a label. */
    PARSER_GROUP_LABEL,
    /** End of file; no semantic group was generated. */
    PARSER_GROUP_EOF
};


/**
 * A structure representing a single group of tokens that form a single semantic unit.
 */
struct parser_group {
    /** The type of the group, used to determine which other members are usable. */
    enum parser_group_type type;
    /** The opcode of an instruction, if type is PARSER_GROUP_INSTRUCTION. */
    enum isa_opcode opcode;
    /** The dest register of an instruction, if type is PARSER_GROUP_INSTRUCTION. */
    uint32_t rd;
    /** The source1 register of an instruction, if type is PARSER_GROUP_INSTRUCTION. */
    uint32_t rs1;
    /** The source2 register of an instruction, if type is PARSER_GROUP_INSTRUCTION. */
    uint32_t rs2;
    /** The immediate/constant value as a number (literal for INSTRUCTION, address for LABEL). */
    uint32_t imm_num;
    /** The immediate/constant value as a symbol (for INSTRUCTION or LABEL). */
    char imm_label[LEXER_TOKEN_MAX_LENGTH + 1];
};


/**
 * A node in a singly-linked list of parser groups.
 */
struct parser_group_node {
    /** The parser group. */
    struct parser_group group;
    /** The binary of the parser group (e.g. machine code of an instruction), set by the encoder. */
    uint32_t binary;
    /** Pointer to the next node. */
    struct parser_group_node *next;
};


/**
 * The status of parser API functions.
 */
enum parser_status {
    /** The parser operation was successful. */
    PARSER_STATUS_SUCCESS = 0,
    /** The parser reached the end of the file. */
    PARSER_STATUS_EOF     = 1,
    /** The parser API function was called with an invalid argument. */
    PARSER_STATUS_INVALID_ARGUMENT,
    /** The parser encountered a semantic error during parsing. */
    PARSER_STATUS_SEMANTIC_ERROR
};


/**
 * Attempts to parse the next semantic group from the provided file.
 *
 * @param file        A file to read tokens from.
 * @param group       A pointer to a non-null group to populate with semantic information.
 * @param last_token  A pointer to store the last lexical token, which can be used to debug
 *                    semantic errors.
 *
 * @return The status of parsing.
 */
enum parser_status parser_next_group(FILE *file,
                                     struct parser_group *group,
                                     struct lexer_token *last_token);


/**
 * Parses all semantic groups from a file, returning them in chronological order as a list.
 *
 * @param file          THe file to read tokens from.
 * @param base_address  The base address for the generated code, used during label resolution.
 *
 * @return Pointer to the first semantic group node.
 */
struct parser_group_node *parser_parse_file(FILE *file, uint16_t base_address);


/**
 * Frees a list of nodes returned by parser_parse_file.
 *
 * @param head  Pointer to the head node of the list to free.
 */
void parser_free_group_nodes(struct parser_group_node *head);


#endif  // _ASSEMBLER_PARSER_H_
