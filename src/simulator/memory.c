#include "simulator/memory.h"
#include "architecture/logger.h"
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>


uint16_t memory_load_halfword(const struct memory *memory, uint16_t address) {
    if (memory == NULL) {
        return 0;
    }
    uint16_t halfword = (memory->m[address + 1] << CHAR_BIT) | memory->m[address];
    log_trace("Load:  M[0x%04" PRIx16 ":0x%04" PRIx16 "] = 0x%04" PRIx16,
              address + 1, address, halfword);
    return halfword;
}


uint8_t memory_load_byte(const struct memory *memory, uint16_t address) {
    if (memory == NULL) {
        return 0;
    }
    uint8_t byte = memory->m[address];
    log_trace("Load:  M[0x%04" PRIx16 "] = 0x%02" PRIx16, address, byte);
    return byte;
}


void memory_store_halfword(struct memory *memory, uint16_t address, uint16_t value) {
    if (memory == NULL) {
        return;
    }
    memory->m[address + 1] = value >> CHAR_BIT;
    memory->m[address] = value & ((1U << CHAR_BIT) - 1U);
    log_trace("Store: M[0x%04" PRIx16 ":0x%04" PRIx16 "] = 0x%04" PRIx16,
              address + 1, address, value);
}


void memory_store_byte(struct memory *memory, uint16_t address, uint8_t value) {
    if (memory == NULL) {
        return;
    }
    memory->m[address] = value;
    log_trace("Store: M[0x%04" PRIx16 "] = 0x%02" PRIx16, address, value);
}
