#include <stdio.h>
#include "simulator/processor.h"


int main(void) {
    struct processor *processor = create_processor();

    FILE *program = fopen("a.out", "rb");
    (void) processor_load_program(processor, program, 0x0100, 0x5F00);
    fclose(program);

    enum processor_status status;
    while ((status = processor_tick(processor)) == PROCESSOR_STATUS_SUCCESS) {
        printf("executed instruction\n");
    }
    printf("final status: %d\n", status);

    destroy_processor(processor);
    return 0;
}
