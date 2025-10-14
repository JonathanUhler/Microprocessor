#include <stdio.h>
#include "simulator/processor.h"
#include "architecture/logger.h"


int main(void) {
    logger_set_level(LOGGER_LEVEL_TRACE);

    struct processor *processor = create_processor();

    FILE *program = fopen("a.out", "rb");
    (void) processor_load_program(processor, program, 0x0100, 0x5F00);
    fclose(program);

    enum processor_status status;
    while ((status = processor_tick(processor)) == PROCESSOR_STATUS_SUCCESS);

    destroy_processor(processor);
    return 0;
}
