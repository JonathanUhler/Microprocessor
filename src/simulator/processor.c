#include "simulator/processor.h"
#include "architecture/isa.h"
#include "architecture/logger.h"
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


struct processor *create_processor(void) {
    struct processor *processor = (struct processor *) malloc(sizeof(struct processor));
    processor->memory = (struct memory *) malloc(sizeof(struct memory));
    processor->registers = (struct register_file *) malloc(sizeof(struct register_file));
    processor->registers->ccount = 0;
    processor_assert_reset(processor);
    return processor;
}


void destroy_processor(struct processor *processor) {
    free(processor->memory);
    free(processor->registers);
    free(processor);
}


enum processor_status processor_load_program(struct processor *processor,
                                             FILE *file,
                                             uint16_t address)
{
    if (processor == NULL || file == NULL) {
        return PROCESSOR_STATUS_INVALID_ARGUMENT;
    }

    uint32_t bytes_read = 0;
    while (true) {
        int byte = fgetc(file);
        if (byte == EOF) {
            break;
        }

        memory_store_byte(processor->memory, address + bytes_read, byte);
        bytes_read++;
        if (address + bytes_read > UINT16_MAX) {
            return PROCESSOR_STATUS_OUT_OF_MEMORY;
        }
    }

    log_info("Loaded %" PRIu32 " bytes into instruction memory at 0x%04" PRIx16,
             bytes_read,
             address);

    return PROCESSOR_STATUS_SUCCESS;
}


enum processor_status processor_assert_reset(struct processor *processor) {
    if (processor == NULL) {
        return PROCESSOR_STATUS_INVALID_ARGUMENT;
    }
    processor->registers->pc = 0x0100;
    processor->registers->reset = 0x0001;
    return PROCESSOR_STATUS_SUCCESS;
}


enum processor_status processor_deassert_reset(struct processor *processor) {
    if (processor == NULL) {
        return PROCESSOR_STATUS_INVALID_ARGUMENT;
    }
    processor->registers->reset = 0x0000;
    return PROCESSOR_STATUS_SUCCESS;
}


static uint32_t processor_fetch_instruction(struct processor *processor) {
    uint16_t start_addr = processor->registers->pc;
    uint16_t end_addr = start_addr + sizeof(uint32_t);

    uint32_t binary = 0;
    for (uint16_t addr = end_addr - 1; addr <= end_addr && addr >= start_addr; addr--) {
        binary <<= CHAR_BIT;
        binary |= memory_load_byte(processor->memory, addr);
    }
    return binary;
}


union isa_instruction processor_decode_instruction(uint32_t binary,
                                                      enum isa_opcode_format *format)
{
    union isa_instruction instruction;
    instruction.binary = binary;
    *format = (enum isa_opcode_format) (binary & ((1U << ISA_INSTRUCTION_FORMAT_SIZE) - 1U));
    return instruction;
}


static enum processor_status processor_execute_i_type(struct processor *processor,
                                                      struct isa_i_format instruction)
{
    enum isa_opcode opcode =
        (enum isa_opcode) ((instruction.funct << ISA_INSTRUCTION_FORMAT_SIZE) | instruction.format);
    uint16_t immediate = instruction.immediate;

    const struct isa_opcode_map *opcode_map = isa_get_opcode_map_from_opcode(opcode);
    log_debug("Execute: %s 0x%04" PRIx16, opcode_map->symbol, immediate);

    switch (opcode) {
    case HALT:
        processor->registers->reset = 0x0001;
        return PROCESSOR_STATUS_HALTED;
    default:
        return PROCESSOR_STATUS_INVALID_INSTRUCTION;
    }
    return PROCESSOR_STATUS_SUCCESS;
}


static enum processor_status processor_execute_dsi_type(struct processor *processor,
                                                        struct isa_dsi_format instruction)
{
    struct memory *memory = processor->memory;
    struct register_file *registers = processor->registers;

    enum isa_opcode opcode =
        (enum isa_opcode) ((instruction.funct << ISA_INSTRUCTION_FORMAT_SIZE) | instruction.format);
    enum isa_register dest = (enum isa_register) instruction.dest;
    enum isa_register source1 = (enum isa_register) instruction.source1;
    uint16_t immediate = instruction.immediate;

    const struct isa_opcode_map *opcode_map = isa_get_opcode_map_from_opcode(opcode);
    log_debug("Execute: %s %s, %s, 0x%04" PRIx16,
              opcode_map->symbol,
              isa_get_register_map_from_index(dest)->symbol,
              isa_get_register_map_from_index(source1)->symbol,
              immediate);

    switch (opcode) {
    case ADDI:
        registers_write(registers, dest, registers_read(registers, source1) + immediate);
        break;
    case SUBI:
        registers_write(registers, dest, registers_read(registers, source1) - immediate);
        break;
    case ANDI:
        registers_write(registers, dest, registers_read(registers, source1) & immediate);
        break;
    case ORI:
        registers_write(registers, dest, registers_read(registers, source1) | immediate);
        break;
    case XORI:
        registers_write(registers, dest, registers_read(registers, source1) ^ immediate);
        break;
    case SLLI:
        registers_write(registers, dest, registers_read(registers, source1) << immediate);
        break;
    case SRLI:
        registers_write(registers, dest, registers_read(registers, source1) >> immediate);
        break;
    case SRAI:
        registers_write(registers, dest, (int16_t) registers_read(registers, source1) >> immediate);
        break;
    case LD: {
        uint16_t addr = registers_read(registers, source1) + immediate;
        registers_write(registers, dest, memory_load_halfword(memory, addr));
        break;
    }
    case ST: {
        uint16_t addr = registers_read(registers, source1) + immediate;
        memory_store_halfword(memory, addr, registers_read(registers, dest));
        break;
    }
    case JL0: {
        if (registers_read(registers, source1) == 0x0000) {
            registers_write(registers, dest, registers->pc + sizeof(uint32_t));
            registers->pc = immediate;
        }
        break;
    }
    case JL1: {
        if (registers_read(registers, source1) == 0x0001) {
            registers_write(registers, dest, registers->pc + sizeof(uint32_t));
            registers->pc = immediate;
        }
        break;
    }
    default:
        return PROCESSOR_STATUS_INVALID_INSTRUCTION;
    }
    return PROCESSOR_STATUS_SUCCESS;
}


static enum processor_status processor_execute_dss_type(struct processor *processor,
                                                        struct isa_dss_format instruction)
{
    struct register_file *registers = processor->registers;

    enum isa_opcode opcode =
        (enum isa_opcode) ((instruction.funct << ISA_INSTRUCTION_FORMAT_SIZE) | instruction.format);
    enum isa_register dest = (enum isa_register) instruction.dest;
    enum isa_register source1 = (enum isa_register) instruction.source1;
    enum isa_register source2 = (enum isa_register) instruction.source2;

    const struct isa_opcode_map *opcode_map = isa_get_opcode_map_from_opcode(opcode);
    log_debug("Execute: %s %s, %s, %s",
              opcode_map->symbol,
              isa_get_register_map_from_index(dest)->symbol,
              isa_get_register_map_from_index(source1)->symbol,
              isa_get_register_map_from_index(source2)->symbol);

    switch (opcode) {
    case ADD:
        registers_write(registers, dest,
                        registers_read(registers, source1) + registers_read(registers, source2));
        break;
    case SUB:
        registers_write(registers, dest,
                        registers_read(registers, source1) - registers_read(registers, source2));
        break;
    case AND:
        registers_write(registers, dest,
                        registers_read(registers, source1) & registers_read(registers, source2));
        break;
    case OR:
        registers_write(registers, dest,
                        registers_read(registers, source1) | registers_read(registers, source2));
        break;
    case XOR:
        registers_write(registers, dest,
                        registers_read(registers, source1) ^ registers_read(registers, source2));
        break;
    case SLL:
        registers_write(registers, dest,
                        registers_read(registers, source1) << registers_read(registers, source2));
        break;
    case SRL:
        registers_write(registers, dest,
                        registers_read(registers, source1) >> registers_read(registers, source2));
        break;
    case SRA:
        registers_write(registers, dest,
                        (int16_t) registers_read(registers, source1) >>
                        registers_read(registers, source2));
        break;
    case EQ:
        registers_write(registers, dest,
                        registers_read(registers, source1) == registers_read(registers, source2));
        break;
    case GT:
        registers_write(registers, dest,
                        registers_read(registers, source1) > registers_read(registers, source2));
        break;
    case LT:
        registers_write(registers, dest,
                        registers_read(registers, source1) < registers_read(registers, source2));
        break;
    case NE:
        registers_write(registers, dest,
                        registers_read(registers, source1) != registers_read(registers, source2));
        break;
    case JLR0: {
        if (registers_read(registers, source1) == 0x0000) {
            registers_write(registers, dest, registers->pc + sizeof(uint32_t));
            registers->pc = registers_read(registers, source2);
        }
        break;
    }
    case JLR1: {
        if (registers_read(registers, source1) == 0x0001) {
            registers_write(registers, dest, registers->pc + sizeof(uint32_t));
            registers->pc = registers_read(registers, source2);
        }
        break;
    }
    default:
        return PROCESSOR_STATUS_INVALID_INSTRUCTION;
    }
    return PROCESSOR_STATUS_SUCCESS;
}


enum processor_status processor_tick(struct processor *processor) {
    if (processor == NULL) {
        return PROCESSOR_STATUS_INVALID_ARGUMENT;
    }
    if (processor->registers->reset == 0x0001) {
        return PROCESSOR_STATUS_HALTED;
    }

    log_info("Clock tick: pc = 0x%04" PRIx16, processor->registers->pc);

    uint32_t binary = processor_fetch_instruction(processor);
    log_debug("Fetch: 0x%08" PRIx32, binary)

    enum isa_opcode_format format;
    union isa_instruction instruction = processor_decode_instruction(binary, &format);

    enum processor_status execute_status;
    uint16_t old_pc = processor->registers->pc;
    switch (format) {
    case ISA_OPCODE_FORMAT_I:
        log_debug("Decode: I-type instruction");
        execute_status = processor_execute_i_type(processor, instruction.i_type);
        break;
    case ISA_OPCODE_FORMAT_DSI:
        log_debug("Decode: DSI-type instruction");
        execute_status = processor_execute_dsi_type(processor, instruction.dsi_type);
        break;
    case ISA_OPCODE_FORMAT_DSS:
        log_debug("Decode: DSS-type instruction");
        execute_status = processor_execute_dss_type(processor, instruction.dss_type);
        break;
    default:
        execute_status = PROCESSOR_STATUS_INVALID_INSTRUCTION;
        break;
    }

    if (execute_status != PROCESSOR_STATUS_SUCCESS) {
        if (execute_status == PROCESSOR_STATUS_HALTED) {
            log_info("Processor halted at pc = 0x%04" PRIx16, processor->registers->pc);
        }
        else {
            log_error("Simulator error after instruction decode: %d", execute_status);
        }
        return execute_status;
    }

    if (processor->registers->pc == old_pc) {
        processor->registers->pc += sizeof(uint32_t);
    }
    processor->registers->ccount++;
    return PROCESSOR_STATUS_SUCCESS;
}
