#ifndef _ASSEMBLER_ENCODER_H_
#define _ASSEMBLER_ENCODER_H_


#define ENCODER_MAX_LABELS 1024


#include "assembler/parser.h"
#include <stdint.h>


enum encoder_status {
    ENCODER_STATUS_SUCCESS = 0,
    ENCODER_STATUS_UNKNOWN_LABEL
};


enum encoder_status encoder_encode_groups(struct parser_group_node **groups);


#endif  // _ASSEMBLER_ENCODER_H_
