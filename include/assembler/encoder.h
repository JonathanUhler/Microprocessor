/**
 * Encoder to convert groups of tokens emitted by the parser into machine code.
 *
 * @author Jonathan Uhler
 */


#ifndef _ASSEMBLER_ENCODER_H_
#define _ASSEMBLER_ENCODER_H_


/** The maximum number of symbolic labels that may be defined. */
#define ENCODER_MAX_LABELS 1024


#include "assembler/parser.h"
#include "structures/list.h"
#include <stdint.h>


/**
 * The status of encoder API functions.
 */
enum encoder_status {
    /** The encoder operation was successful. */
    ENCODER_STATUS_SUCCESS = 0,
    /** The encoder encountered an undefined label. */
    ENCODER_STATUS_UNKNOWN_LABEL
};


/**
 * Encodes the provided list of parser token groups into machine code.
 *
 * Encoding is performed in two steps:
 *
 *   1) The encoder will find all groups in the provided list with the type PARSER_GROUP_LABEL.
 *      IMPORTANT: Label nodes will be REMOVED from the provided list by the encoder (and later
 *      freed by the encoder)--these nodes will not exist in the node list after encoding.
 *   2) The encoder will process the remaining PARSER_GROUP_INSTRUCTION nodes and set the
 *      .binary member of each node to the encoded binary. The .imm_num member of each parser group
 *      (within a node) may be set during label resolution/encoding.
 *
 * After encoding, the caller is still responsible for using the list API to free the nodes left
 * in the provided list.
 *
 * @param groups  List of parser group nodes to encode.
 *
 * @return Whether encoding was successful. If SUCCESS, the encoded binary can be read by traversing
 *         the groups list and reading the .binary member of each instruction node. On error, the
 *         state/order of nodes in the groups list is not guaranteed. In either case, the caller is
 *         still responsible for freeing groups with the list API.
 */
enum encoder_status encoder_encode_groups(struct list *groups);


#endif  // _ASSEMBLER_ENCODER_H_
