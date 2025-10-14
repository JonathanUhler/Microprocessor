/**
 * Memory I/O for the single-cycle simulator.
 *
 * @author Jonathan Uhler
 */


#ifndef _SIMULATOR_MEMORY_H_
#define _SIMULATOR_MEMORY_H_


#include <stdint.h>


/**
 * The structure of memory.
 */
struct memory {
    uint8_t m[65536];
};


/**
 * Loads a 16-bit halfword from the specified absolute address in memory.
 *
 * @param memory   The memory to load from.
 * @param address  The address to load from (which is the address of the lower byte).
 *
 * @return The loaded halfword.
 */
uint16_t memory_load_halfword(const struct memory *memory, uint16_t address);


/**
 * Loads an 8-bit byte from the specified absolute address in memory.
 *
 * @param memory   The memory to load from.
 * @param address  The address to load from.
 *
 * @return The loaded byte.
 */
uint8_t memory_load_byte(const struct memory *memory, uint16_t address);


/**
 * Stores a 16-bit halfword into the specified absolute address in memory.
 *
 * @param memory   The memory to store to.
 * @param address  The address to write to (which is the address of the lower byte).
 * @param value    The halfword to write.
 */
void memory_store_halfword(struct memory *memory, uint16_t address, uint16_t value);


/**
 * Stores an 8-bit byte into the specified absolute address in memory.
 *
 * @param memory   The memory to store to.
 * @param address  The address to write to.
 * @param value    The byte to write.
 */
void memory_store_byte(struct memory *memory, uint16_t address, uint8_t value);


#endif  // _SIMULATOR_MEMORY_H_
