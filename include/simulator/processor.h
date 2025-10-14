/**
 * Interface for a basic single-cycle simulator.
 *
 * @author Jonathan Uhler
 */


#ifndef _SIMULATOR_PROCESSOR_H_
#define _SIMULATOR_PROCESSOR_H_


#include "simulator/memory.h"
#include "simulator/registers.h"
#include <stdint.h>
#include <stdio.h>


/**
 * The processor state.
 */
struct processor {
    /** Memory in use by the processor. */
    struct memory *memory;
    /** Register file in ues by the processor. */
    struct register_file *registers;
};


/**
 * The status of processor API functions.
 */
enum processor_status {
    /** The processor API function completed successfully. */
    PROCESSOR_STATUS_SUCCESS = 0,
    /** The processor is halted (reset is asserted). */
    PROCESSOR_STATUS_HALTED  = 1,
    /** The processor API function was called with an invalid argument. */
    PROCESSOR_STATUS_INVALID_ARGUMENT,
    /** The processor encountered an invalid memory address. */
    PROCESSOR_STATUS_INVALID_ADDRESS,
    /** The processor attempted to load an invalid instruction. */
    PROCESSOR_STATUS_INVALID_INSTRUCTION,
    /** The processor ran out of memory. */
    PROCESSOR_STATUS_OUT_OF_MEMORY
};


/**
 * Creates a new processor.
 *
 * The caller is responsible for calling destroy_processor to free associated memory.
 *
 * @return Pointer to the created processor.
 */
struct processor *create_processor(void);


/**
 * Frees a processor structure allocated with create_processor.
 *
 * @param processor  Pointer to the processor to destroy.
 */
void destroy_processor(struct processor *processor);


/**
 * Loads a program binary file into processor memory at the specified address.
 *
 * @param processor  The processor to load the program into.
 * @param file       The program to load.
 * @param address    The base address to begin loading at.
 *
 * @return Whether loading was successful.
 */
enum processor_status processor_load_program(struct processor *processor,
                                             FILE *file,
                                             uint16_t address,
                                             uint16_t region_size);


/**
 * Steps the processor clock forward by one cycle.
 *
 * @param processor  The processor to step.
 */
enum processor_status processor_tick(struct processor *processor);


#endif  // _SIMULATOR_PROCESSOR_H_
