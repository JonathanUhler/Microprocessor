#ifndef _SIMULATOR_REGISTERS_H_
#define _SIMULATOR_REGISTERS_H_


#include "architecture/isa.h"
#include <stdint.h>


struct register_file {
    uint16_t pc;
    uint16_t reset;
    uint16_t gp[R31 - R0 + 1];
};


uint16_t registers_read(const struct register_file *rf, enum isa_register index);


void registers_write(struct register_file *rf, enum isa_register index, uint16_t value);


#endif  // _SIMULATOR_REGISTERS_H_
