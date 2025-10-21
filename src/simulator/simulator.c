#include "simulator/cli.h"
#include "simulator/processor.h"
#include "architecture/logger.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>


int main(void) {    
    logger_set_level(LOGGER_LEVEL_WARN);

    struct processor *processor = create_processor();
    cli_run(processor);
    destroy_processor(processor);

    return 0;
}
