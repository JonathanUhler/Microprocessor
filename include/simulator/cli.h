#ifndef _SIMULATOR_CLI_H_
#define _SIMULATOR_CLI_H_


#include "simulator/processor.h"


#define CLI_MAX_COMMAND_LENGTH    128
#define CLI_MAX_COMMAND_ARGUMENTS  32

#define CLI_INFO_MEMORY_BYTES_PER_ROW   16
#define CLI_INFO_MEMORY_BYTES_PER_GROUP  2


struct cli_command_descriptor {
    const char *name;
    void (*handler)(struct processor *processor, int argc, char **argv);
    const char *args_help;
    const char *help;
};


void cli_run(struct processor *processor);


#endif  // _SIMULATOR_CLI_H_
