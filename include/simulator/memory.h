#ifndef _SIMULATOR_MEMORY_H_
#define _SIMULATOR_MEMORY_H_


#include <stdint.h>


struct memory_default_map {
    uint8_t __GUARD__[256];
    uint8_t vectors[256];
    uint8_t code[24064];
    uint8_t allocable[40958];
};


struct memory {
    uint8_t m[65536];
};


uint16_t memory_load_halfword(const struct memory *memory, uint16_t address);


uint8_t memory_load_byte(const struct memory *memory, uint16_t address);


void memory_store_halfword(struct memory *memory, uint16_t address, uint16_t value);


void memory_store_byte(struct memory *memory, uint16_t address, uint8_t value);


#endif  // _SIMULATOR_MEMORY_H_
