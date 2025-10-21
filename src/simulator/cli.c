#include "simulator/cli.h"
#include "architecture/logger.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


static void cli_process_help(struct processor *processor, int argc, char **argv);
static void cli_process_break(struct processor *processor, int argc, char **argv);
static void cli_process_continue(struct processor *processor, int argc, char **argv);
static void cli_process_load(struct processor *processor, int argc, char **argv);
static void cli_process_quit(struct processor *processor, int argc, char **argv);
static void cli_process_show_break(struct processor *processor, int argc, char **argv);
static void cli_process_show_memory(struct processor *processor, int argc, char **argv);
static void cli_process_show_registers(struct processor *processor, int argc, char **argv);
static void cli_process_start(struct processor *processor, int argc, char **argv);
static void cli_process_tick(struct processor *processor, int argc, char **argv);
static void cli_process_unbreak(struct processor *processor, int argc, char **argv);


static const struct cli_command_descriptor cli_command_table[] = {
    {"help", cli_process_help, NULL, "print command help information"},
    {"break", cli_process_break, "<address>", "add a breakpoint at address"},
    {"continue", cli_process_continue, NULL,
     "continue until reset is asserted, an error occurs, or a breakpoint is hit"},
    {"load", cli_process_load, "<file> <address>", "load contents of binary file at address"},
    {"quit", cli_process_quit, NULL, "exit the simulator"},
    {"show break", cli_process_show_break, NULL, "show all breakpoint numbers and their addresses"},
    {"show memory", cli_process_show_memory, "[[start:]end ...]", "show contents of main memory"},
    {"show registers", cli_process_show_registers, "[name ...]", "show contents of registers"},
    {"start", cli_process_start, NULL, "assert and deassert reset to cycle the simulated core"},
    {"tick", cli_process_tick, "[cycles]", "tick the clock by specified amount"},
    {"unbreak", cli_process_unbreak, "<breakpoint>", "remove a breakpoint"}
};


static void cli_process_help(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;

    size_t num_commands = sizeof(cli_command_table) / sizeof(cli_command_table[0]);
    for (size_t i = 0; i < num_commands; i++) {
        struct cli_command_descriptor descriptor = cli_command_table[i];
        printf("\033[1m%s", descriptor.name);
        if (descriptor.args_help != NULL) {
            printf(" %s", descriptor.args_help);
        }
        printf("\033[0m -- %s\n", descriptor.help);
    }
}


static void cli_process_break(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


static void cli_process_continue(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


static void cli_process_load(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


static void cli_process_quit(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


static void cli_process_show_break(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


static void cli_process_show_memory(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


static void cli_process_show_registers(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


static void cli_process_start(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


static void cli_process_tick(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


static void cli_process_unbreak(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


void cli_run(struct processor *processor) {
    (void) processor;

    while (true) {
        printf("(sim) ");
        fflush(stdout);

        char command[CLI_MAX_COMMAND_LENGTH];
        char *result = fgets(command, CLI_MAX_COMMAND_LENGTH, stdin);
        if (result == NULL) {
            log_fatal("Could not read command from standard input");
        }
    }
}
