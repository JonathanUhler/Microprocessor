#include "simulator/registers.h"
#include "architecture/isa.h"
#include <stdint.h>
#include <stdlib.h>


uint16_t registers_read(const struct register_file *rf, enum isa_register index) {
    if (rf == NULL) {
        return 0;
    }
    if (index == ZERO) {
        return 0;
    }
    return rf->gp[index];
}


void registers_write(struct register_file *rf, enum isa_register index, uint16_t value) {
    if (rf == NULL) {
        return;
    }
    if (index == ZERO) {
        return;
    }
    rf->gp[index] = value;
}
