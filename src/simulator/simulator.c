#include <stdio.h>
#include "simulator/processor.h"
#include "architecture/logger.h"


int main(void) {
    logger_set_level(LOGGER_LEVEL_TRACE);

    struct processor *processor = create_processor();

    FILE *program = fopen("a.out", "rb");
    (void) processor_load_program(processor, program, 0x0100);
    fclose(program);

    processor_deassert_reset(processor);

    enum processor_status status = PROCESSOR_STATUS_SUCCESS;
    uint32_t total_ccount = 0;

    while (status == PROCESSOR_STATUS_SUCCESS) {
        uint16_t old_ccount = processor->registers->ccount;
        status = processor_tick(processor);
        uint16_t new_ccount = processor->registers->ccount;

        if (new_ccount < old_ccount) {
            total_ccount++;
        }
        else {
            total_ccount += (new_ccount - old_ccount);
        }
    }

    printf("Executed %d instructions\n", total_ccount);

    destroy_processor(processor);
    return 0;
}
