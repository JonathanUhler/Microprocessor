#include "assembler/encoder.h"
#include "assembler/logger.h"
#include "assembler/parser.h"
#include "architecture/isa.h"
#include <stdint.h>


static enum encoder_status encoder_resolve_labels(struct parser_group_node **groups) {
    struct parser_group_node *labels = NULL;
    struct parser_group_node *curr_label = NULL;

    struct parser_group_node *head = *groups;
    struct parser_group_node *prev = NULL;
    struct parser_group_node *curr = *groups;
    while (curr != NULL) {
        struct parser_group group = curr->group;
        if (group.type != PARSER_GROUP_LABEL) {
            prev = curr;
            curr = curr->next;
            continue;
        }

        if (curr_label == NULL) {
            labels = curr;
            curr_label = curr;
        }
        else {
            curr_label->next = curr;
            curr_label = curr;
        }

        struct parser_group_node *next = curr->next;
        if (prev != NULL) {
            prev->next = next;
        }
        else {
            head = next;
        }
        curr = next;
        curr_label->next = NULL;
    }

    curr = head;
    while (curr != NULL) {
        if (curr->group.imm_label[0] == '\0') {
            curr = curr->next;
            continue;
        }

        curr_label = labels;
        while (curr_label != NULL) {
            if (strncmp(curr_label->group.imm_label,
                        curr->group.imm_label,
                        LEXER_TOKEN_MAX_LENGTH) == 0)
            {
                curr->group.imm_num = curr_label->group.imm_num;
                break;
            }
            curr_label = curr_label->next;
        }
        if (curr_label == NULL) {
            log_error("use of undeclared label '%s'", curr->group.imm_label);
            return ENCODER_STATUS_UNKNOWN_LABEL;
        }

        curr = curr->next;
    }

    parser_free_group_nodes(labels);
    *groups = head;
    return ENCODER_STATUS_SUCCESS;
}


enum encoder_status encoder_encode_groups(struct parser_group_node **groups) {
    enum encoder_status label_resolution_status = encoder_resolve_labels(groups);
    if (label_resolution_status != ENCODER_STATUS_SUCCESS) {
        return label_resolution_status;
    }

    struct parser_group_node *curr = *groups;
    while (curr != NULL) {
        struct parser_group group = curr->group;
        if (group.type != PARSER_GROUP_INSTRUCTION) {
            curr = curr->next;
            continue;
        }

        const struct isa_opcode_map *opcode_map = isa_get_opcode_map_from_opcode(group.opcode);
        if (opcode_map == NULL) {
            curr = curr->next;
            continue;
        }

        union isa_instruction instruction;
        if (opcode_map->format == ISA_OPCODE_FORMAT_I) {
            instruction.i_type = (struct isa_i_format) {.format = opcode_map->format,
                                                        .funct = opcode_map->funct,
                                                        .immediate = group.imm_num};
        }
        else if (opcode_map->format == ISA_OPCODE_FORMAT_DSI) {
            instruction.dsi_type = (struct isa_dsi_format) {.format = opcode_map->format,
                                                            .funct = opcode_map->funct,
                                                            .dest = group.rd,
                                                            .source1 = group.rs1,
                                                            .immediate = group.imm_num};
        }
        else if (opcode_map->format == ISA_OPCODE_FORMAT_DSS) {
            instruction.dss_type = (struct isa_dss_format) {.format = opcode_map->format,
                                                            .funct = opcode_map->funct,
                                                            .dest = group.rd,
                                                            .source1 = group.rs1,
                                                            .source2 = group.rs2};
        }
        else {
            curr = curr->next;
            continue;
        }

        curr->binary = instruction.binary;
        curr = curr->next;
    }

    return ENCODER_STATUS_SUCCESS;
}

