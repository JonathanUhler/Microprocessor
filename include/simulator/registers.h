#ifndef _SIMULATOR_REGISTERS_H_
#define _SIMULATOR_REGISTERS_H_


#include "assembler/lexer.h"


struct register_file {
    uint16_t pc;
    uint16_t reset;
    uint16_t gp[R31 - R0 + 1];
};


uint16_t registers_read(const struct register_file *rf, enum lexer_register reg);


void registers_write(struct register_file *rf, enum lexer_register reg, uint16_t value);


#endif  // _SIMULATOR_REGISTERS_H_
