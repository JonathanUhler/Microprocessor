#ifndef _SIMULATOR_PROCESSOR_H_
#define _SIMULATOR_PROCESSOR_H_


#include "simulator/memory.h"
#include "simulator/registers.h"
#include <stdint.h>
#include <stdio.h>


struct processor {
    struct memory *memory;
    struct register_file *registers;
};


enum processor_status {
    PROCESSOR_STATUS_SUCCESS = 0,
    PROCESSOR_STATUS_HALTED  = 1,
    PROCESSOR_STATUS_INVALID_ARGUMENT,
    PROCESSOR_STATUS_INVALID_ADDRESS,
    PROCESSOR_STATUS_INVALID_INSTRUCTION,
    PROCESSOR_STATUS_OUT_OF_MEMORY
};


struct processor *create_processor(void);


void destroy_processor(struct processor *processor);


enum processor_status processor_load_program(struct processor *processor,
                                             FILE *file,
                                             uint16_t address,
                                             uint16_t region_size);


enum processor_status processor_tick(struct processor *processor);


#endif  // _SIMULATOR_PROCESSOR_H_
