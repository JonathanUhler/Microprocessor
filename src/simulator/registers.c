#include "simulator/registers.h"
#include "architecture/isa.h"
#include "architecture/logger.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>


uint16_t registers_read(const struct register_file *rf, enum isa_register index) {
    if (rf == NULL) {
        return 0;
    }
    uint16_t value = rf->gp[index];
    log_trace("Read:  R[%2d] = 0x%04" PRIx16, index, value);
    return value;
}


void registers_write(struct register_file *rf, enum isa_register index, uint16_t value) {
    if (rf == NULL) {
        return;
    }
    if (index == ZERO) {
        return;
    }
    rf->gp[index] = value;
    log_trace("Write: R[%2d] = 0x%04" PRIx16, index, value);
}
