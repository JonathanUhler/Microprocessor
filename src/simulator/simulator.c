#include <stdio.h>
#include "simulator/processor.h"


int main(void) {
    struct processor *processor = create_processor();

    FILE *program = fopen("a.out", "rb");
    (void) processor_load_program(processor, program, 0x0100, 0x5F00);
    fclose(program);

    printf("--MEMORY--");
    for (uint16_t addr = 0x0100; addr < 0x0140; addr++) {
        if ((addr % 4 == 0)) {
            printf("\n%04x:  ", addr);
        }
        printf("%02x", memory_load_byte(processor->memory, addr));
        if (addr % 2 == 1) {
            printf(" ");
        }
    }
    printf("\n----------\n");

    enum processor_status status;
    while ((status = processor_tick(processor)) == PROCESSOR_STATUS_SUCCESS) {
        printf("executed instruction\n");
    }
    printf("final status: %d\n", status);

    destroy_processor(processor);
    return 0;
}
