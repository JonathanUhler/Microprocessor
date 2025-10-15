/**
 * Register I/O for the single-cycle simulator.
 *
 * @author Jonathan Uhler
 */


#ifndef _SIMULATOR_REGISTERS_H_
#define _SIMULATOR_REGISTERS_H_


#include "architecture/isa.h"
#include <stdint.h>


/**
 * All the regsiters (general purpose and CSRs) in the processor.
 */
struct register_file {
    /** Program counter register. */
    uint16_t pc;
    /** Reset signal register. */
    uint16_t reset;
    /** Cycle count (modulo 2^16) register. */
    uint16_t ccount;
    /** General purpose registers. */
    uint16_t gp[R31 - R0 + 1];
};


/**
 * Reads a value from a general purpose register.
 *
 * @param rf     Pointer to the register file to read.
 * @param index  The register index.
 *
 * @return The value in the register.
 */
uint16_t registers_read(const struct register_file *rf, enum isa_register index);


/**
 * Writes a value to a general purpose register.
 *
 * Writes to the ZERO (R0) register will be ignored and will not change the value in the register.
 *
 * @param rf     Pointer to the register file to write.
 * @param index  The register index.
 * @param value  The value to write.
 */
void registers_write(struct register_file *rf, enum isa_register index, uint16_t value);


#endif  // _SIMULATOR_REGISTERS_H_
