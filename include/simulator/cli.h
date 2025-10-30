/**
 * Command line interface for the simulator.
 *
 * @author Jonathan Uhler
 */


#ifndef _SIMULATOR_CLI_H_
#define _SIMULATOR_CLI_H_


#include "simulator/processor.h"


#define CLI_MAX_COMMAND_LENGTH    128
#define CLI_MAX_COMMAND_ARGUMENTS  32

#define CLI_INFO_MEMORY_BYTES_PER_ROW   16
#define CLI_INFO_MEMORY_BYTES_PER_GROUP  2


/**
 * Description of a command registered with the CLI.
 */
struct cli_command_descriptor {
    /** The name of the command. */
    const char *name;
    /** A callback routine to process the command. */
    void (*handler)(struct processor *processor, int argc, char **argv);
    /** Help message for the command parameters. */
    const char *args_help;
    /** Help message describing the purpose of the command. */
    const char *help;
};


/**
 * Runs a CLI read-eval-print loop on the provided processor.
 *
 * @param processor  The processor to attach a command-line interface to.
 */
void cli_run(struct processor *processor);


#endif  // _SIMULATOR_CLI_H_
