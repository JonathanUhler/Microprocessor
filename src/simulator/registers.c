#include "simulator/registers.h"
#include <stdint.h>
#include <stdlib.h>


uint16_t registers_read(const struct register_file *rf, enum lexer_register reg) {
    if (rf == NULL) {
        return 0;
    }
    if (reg == ZERO) {
        return 0;
    }
    return rf->gp[reg];
}


void registers_write(struct register_file *rf, enum lexer_register reg, uint16_t value) {
    if (rf == NULL) {
        return;
    }
    if (reg == ZERO) {
        return;
    }
    rf->gp[reg] = value;
}
