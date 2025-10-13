#include "simulator/memory.h"
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>


uint16_t memory_load_halfword(const struct memory *memory, uint16_t address) {
    if (memory == NULL) {
        return 0;
    }
    return (memory->m[address + 1] << CHAR_BIT) | memory->m[address];
}


uint8_t memory_load_byte(const struct memory *memory, uint16_t address) {
    if (memory == NULL) {
        return 0;
    }
    return memory->m[address];
}


void memory_store_halfword(struct memory *memory, uint16_t address, uint16_t value) {
    if (memory == NULL) {
        return;
    }
    memory->m[address + 1] = value >> CHAR_BIT;
    memory->m[address] = value & ((1U << CHAR_BIT) - 1U);
}


void memory_store_byte(struct memory *memory, uint16_t address, uint8_t value) {
    if (memory == NULL) {
        return;
    }
    memory->m[address] = value;
}
