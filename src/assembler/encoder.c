#include "assembler/encoder.h"
#include "assembler/parser.h"
#include "architecture/isa.h"
#include "architecture/logger.h"
#include "structures/list.h"
#include <stdint.h>


static enum encoder_status encoder_resolve_labels(struct list *groups) {
    struct list *labels = create_list();

    for (uint32_t i = 0; i < groups->size; i++) {
        void *data;
        list_peek_at(groups, i, &data);
        struct parser_group *group = (struct parser_group *) data;
        if (group->type == PARSER_GROUP_LABEL) {
            list_pop_at(groups, i, &data);
            i--;
            list_add(labels, data);
        }
    }

    for (uint32_t i = 0; i < groups->size; i++) {
        void *data;
        list_peek_at(groups, i, &data);
        struct parser_group *group = (struct parser_group *) data;

        uint32_t l;
        for (l = 0; l < labels->size; l++) {
            void *label_data;
            list_peek_at(labels, i, &label_data);
            struct parser_group *label = (struct parser_group *) label_data;

            if (strncmp(label->label.label, group->instruction.label, LEXER_TOKEN_MAX_LENGTH) == 0)
            {
                group->instruction.immediate = label->label.immediate;
                break;
            }
        }

        if (l >= groups->size) {
            log_error("use of undeclared label '%s'", group->instruction.label);
            return ENCODER_STATUS_UNKNOWN_LABEL;
        }
    }

    destroy_list(labels, &list_default_node_free_callback);
    return ENCODER_STATUS_SUCCESS;
}


enum encoder_status encoder_encode_groups(struct list *groups) {
    enum encoder_status label_resolution_status = encoder_resolve_labels(groups);
    if (label_resolution_status != ENCODER_STATUS_SUCCESS) {
        return label_resolution_status;
    }

    for (uint32_t i = 0; i < groups->size; i++) {
        void *data;
        list_peek_at(groups, i, &data);
        struct parser_group *group = (struct parser_group *) data;
        if (group->type != PARSER_GROUP_INSTRUCTION) {
            continue;
        }

        const struct isa_opcode_map *opcode_map =
            isa_get_opcode_map_from_opcode(group->instruction.opcode);
        if (opcode_map == NULL) {
            continue;
        }

        union isa_instruction instruction;
        switch (opcode_map->format) {
        case ISA_OPCODE_FORMAT_I:
            instruction.i_type = (struct isa_i_format) {
                .format = opcode_map->format,
                .funct = opcode_map->funct,
                .immediate = group->instruction.immediate
            };
            break;
        case ISA_OPCODE_FORMAT_DSI:
            instruction.dsi_type = (struct isa_dsi_format) {
                .format = opcode_map->format,
                .funct = opcode_map->funct,
                .dest = group->instruction.dest,
                .source1 = group->instruction.source1,
                .immediate = group->instruction.immediate
            };
            break;
        case ISA_OPCODE_FORMAT_DSS:
            instruction.dss_type = (struct isa_dss_format) {
                .format = opcode_map->format,
                .funct = opcode_map->funct,
                .dest = group->instruction.dest,
                .source1 = group->instruction.source1,
                .source2 = group->instruction.source2
            };
            break;
        default:
            continue;
        }

        group->instruction.binary = instruction.binary;
    }

    return ENCODER_STATUS_SUCCESS;
}

