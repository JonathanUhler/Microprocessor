#include "assembler/encoder.h"
#include "assembler/parser.h"
#include "architecture/isa.h"
#include "architecture/logger.h"
#include "structures/list.h"
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>


static enum encoder_status encoder_resolve_labels(struct list *groups) {
    struct list *labels = create_list();

    for (uint32_t i = 0; i < groups->size; i++) {
        void *data;
        list_peek_at(groups, i, &data);
        struct parser_group *group = (struct parser_group *) data;
        if (group->type == PARSER_GROUP_LABEL) {
            log_trace("Encoder registered a new label '%s'", group->label.label);
            list_pop_at(groups, i, &data);
            i--;
            list_add(labels, data);
        }
    }
    log_debug("Encoder registered %" PRIu32 " labels", labels->size);

    for (uint32_t i = 0; i < groups->size; i++) {
        void *data;
        list_peek_at(groups, i, &data);
        struct parser_group *group = (struct parser_group *) data;

        uint32_t l;
        for (l = 0; l < labels->size; l++) {
            void *label_data;
            list_peek_at(labels, l, &label_data);
            struct parser_group *label = (struct parser_group *) label_data;

            if (strncmp(label->label.label, group->instruction.label, LEXER_TOKEN_MAX_LENGTH) == 0)
            {
                group->instruction.immediate = label->label.immediate;
                log_trace("Encoder resolved label '%s' to 0x%04" PRIx16,
                          group->instruction.label, group->instruction.immediate);
                break;
            }
        }

        if (l >= groups->size) {
            log_error("Use of undeclared label '%s'", group->instruction.label);
            return ENCODER_STATUS_UNKNOWN_LABEL;
        }
    }

    destroy_list(labels, &list_default_node_free_callback);
    return ENCODER_STATUS_SUCCESS;
}


static enum encoder_status encoder_resolve_instructions(struct list *groups) {
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
            log_error("Encoder could not resolve instruction opcode %d", group->instruction.opcode);
            return ENCODER_STATUS_UNEXPECTED_GROUP;
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


static void encoder_convert_instruction(struct list *bytes, struct parser_group *instruction) {
    for (uint32_t b = 0; b < sizeof(uint32_t); b++) {
        uint8_t *byte = (uint8_t *) malloc(sizeof(uint8_t));
        *byte = (instruction->instruction.binary >> (b * CHAR_BIT)) & UINT8_MAX;
        list_add(bytes, (void *) byte);
        log_trace("Encoder added instruction[%" PRIu32 "] = %02" PRIx8, b, *byte);
    }
}


static void encoder_convert_directive(struct list *bytes, struct parser_group *directive) {
    switch (directive->directive.type) {
    case PARSER_DIRECTIVE_LOC:
        for (uint32_t b = 0; b < directive->directive.loc.num_pad_bytes; b++) {
            uint8_t *byte = (uint8_t *) calloc(1, sizeof(uint8_t));
            list_add(bytes, (void *) byte);
        }
        break;
    case PARSER_DIRECTIVE_HALF:   
        for (uint32_t b = 0; b < sizeof(uint16_t); b++) {
            uint8_t *byte = (uint8_t *) malloc(sizeof(uint8_t));
            *byte = (directive->directive.half.element >> (b * CHAR_BIT)) & UINT8_MAX;
            list_add(bytes, (void *) byte);
        }
        break;
    }
}


enum encoder_status encoder_encode_groups(struct list *groups, struct list **bytes) {
    enum encoder_status label_resolution_status = encoder_resolve_labels(groups);
    if (label_resolution_status != ENCODER_STATUS_SUCCESS) {
        return label_resolution_status;
    }

    enum encoder_status instruction_resolution_status = encoder_resolve_instructions(groups);
    if (instruction_resolution_status != ENCODER_STATUS_SUCCESS) {
        return instruction_resolution_status;
    }

    *bytes = create_list();

    for (uint32_t i = 0; i < groups->size; i++) {
        void *data;
        list_peek_at(groups, i, &data);
        struct parser_group *group = (struct parser_group *) data;

        switch (group->type) {
        case PARSER_GROUP_INSTRUCTION:
            log_debug("Encoder found instruction group");
            encoder_convert_instruction(*bytes, group);
            break;
        case PARSER_GROUP_DIRECTIVE:
            log_debug("Encoder found directive group");
            encoder_convert_directive(*bytes, group);
            break;
        default:
            log_fatal("Encoder found unexpected semantic group of type %d", group->type);
            break;
        }
    }

    log_info("Encoder finished successfully (bytes encoded: %" PRIu32 ")", (*bytes)->size);
    return ENCODER_STATUS_SUCCESS;
}
