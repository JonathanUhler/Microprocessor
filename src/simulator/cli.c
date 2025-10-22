#include "simulator/cli.h"
#include "architecture/logger.h"
#include "architecture/isa.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


static void cli_process_help(struct processor *processor, int argc, char **argv);
static void cli_process_break(struct processor *processor, int argc, char **argv);
static void cli_process_continue(struct processor *processor, int argc, char **argv);
static void cli_process_load(struct processor *processor, int argc, char **argv);
static void cli_process_quit(struct processor *processor, int argc, char **argv);
static void cli_process_info_break(struct processor *processor, int argc, char **argv);
static void cli_process_info_memory(struct processor *processor, int argc, char **argv);
static void cli_process_info_registers(struct processor *processor, int argc, char **argv);
static void cli_process_start(struct processor *processor, int argc, char **argv);
static void cli_process_tick(struct processor *processor, int argc, char **argv);
static void cli_process_unbreak(struct processor *processor, int argc, char **argv);
static void cli_process_verbose(struct processor *processor, int argc, char **argv);


static const struct cli_command_descriptor cli_command_table[] = {
    {"help", cli_process_help, NULL, "print command help information"},
    {"break", cli_process_break, "<address>", "add a breakpoint at address"},
    {"continue", cli_process_continue, NULL,
     "continue until reset is asserted, an error occurs, or a breakpoint is hit"},
    {"load", cli_process_load, "<file> <address>", "load contents of binary file at address"},
    {"quit", cli_process_quit, NULL, "exit the simulator"},
    {"info break", cli_process_info_break, NULL, "show all breakpoint numbers and their addresses"},
    {"info memory", cli_process_info_memory, "[[start:]end ...]", "show contents of main memory"},
    {"info registers", cli_process_info_registers, "[name ...]", "show contents of registers"},
    {"start", cli_process_start, NULL, "assert and deassert reset to cycle the simulated core"},
    {"tick", cli_process_tick, "[cycles]", "tick the clock by specified amount"},
    {"unbreak", cli_process_unbreak, "<breakpoint>", "remove a breakpoint"},
    {"verbose", cli_process_verbose, "[level]", "set or view level of debug messages"}
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
    (void) argv;

    if (argc != 0) {
        log_error("Unexpected arguments");
        return;
    }

    if (processor->registers->reset == 0x001) {
        log_warn("Reset is asserted, not ticking clock");
        return;
    }

    uint32_t cycles = 0;
    while (processor->registers->reset == 0x0000) {
        enum processor_status tick_status = processor_tick(processor);  // MARK: add breakpoint detection
        if (tick_status != PROCESSOR_STATUS_SUCCESS) {
            log_warn("Execution stopped after %" PRIu32 " cycles (errno %d)", cycles, tick_status);
            return;
        }
        cycles++;
    }
}


static void cli_process_load(struct processor *processor, int argc, char **argv) {
    if (argc != 2) {
        log_error("Unexpected arguments");
        return;
    }

    FILE *program = fopen(argv[0], "rb");
    uint16_t address = strtoul(argv[1], NULL, 0);
    enum processor_status load_status = processor_load_program(processor, program, address);
    if (load_status != PROCESSOR_STATUS_SUCCESS) {
        log_error("Could not load file into memory (errno %d)", load_status);
    }
    if (program != NULL) {
        fclose(program);
    }
}


static void cli_process_quit(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;

    exit(0);
}


static void cli_process_info_break(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


static void cli_process_info_memory(struct processor *processor, int argc, char **argv) {
    if (argc == 0) {
        argc = 1;
        argv[0] = "0x0000:0xFFFF";
    }

    for (uint32_t i = 0; i < (uint32_t) argc; i++) {
        char *range = argv[i];
        uint16_t start;
        uint16_t end;

        char *colon = strchr(range, ':');
        if (colon == NULL) {
            end = strtoul(range, NULL, 0);
            start = end;
        }
        else {
            start = strtoul(range, NULL, 0);
            end = strtoul(colon + 1, NULL, 0);
        }

        for (uint32_t addr = start; addr <= end; addr++) {
            uint8_t byte = memory_load_byte(processor->memory, addr);
            printf("%02" PRIu8, byte);
        }
    }
}


static void cli_process_info_registers(struct processor *processor, int argc, char **argv) {
    if (argc == 0) {
        for (uint32_t i = R0; i < R31; i++) {
            const struct isa_register_map *map = isa_get_register_map_from_index(i);
            uint16_t value = registers_read(processor->registers, map->index);
            printf("%s = 0x%04" PRIx16 "\n", map->symbol, value);
        }

        argc = 3;
        argv[0] = "reset";
        argv[1] = "pc";
        argv[2] = "ccount";
    }

    for (uint32_t i = 0; i < (uint32_t) argc; i++) {
        char *symbol = argv[i];
        uint16_t value = 0;

        if (strcmp(symbol, "reset") == 0) {
            value = processor->registers->reset;
        }
        else if (strcmp(symbol, "pc") == 0) {
            value = processor->registers->pc;
        }
        else if (strcmp(symbol, "ccount") == 0) {
            value = processor->registers->ccount;
        }
        else {
            const struct isa_register_map *map = isa_get_register_map_from_symbol(symbol);
            if (map == NULL) {
                log_error("Unknown register name %s", symbol);
                continue;
            }
            value = registers_read(processor->registers, map->index);
        }

        printf("%s = 0x%04" PRIx16 "\n", symbol, value);
    }
}


static void cli_process_start(struct processor *processor, int argc, char **argv) {
    (void) argv;

    if (argc != 0) {
        log_error("Unexpected arguments")
        return;
    }

    if (processor->registers->reset == 0x0000) {
        printf("Simulation already running. Restart from beginning? (y/n) ");
        char answer[CLI_MAX_COMMAND_LENGTH];
        char *result = fgets(answer, sizeof(answer), stdin);
        if (result == NULL) {
            log_fatal("Could not read answer from standard input");
        }

        if (strcmp(answer, "y\n") != 0) {
            return;
        }
    }

    processor_assert_reset(processor);
    processor_deassert_reset(processor);

    printf("Simulation started. Execution paused at 0x%04" PRIx16 "\n", processor->registers->pc);
}


static void cli_process_tick(struct processor *processor, int argc, char **argv) {
    uint32_t num_cycles;
    switch (argc) {
    case 0:
        num_cycles = 1;
        break;
    case 1:
        num_cycles = atoi(argv[0]);
        break;
    default:
        log_error("Unexpected arguments");
        return;
    }

    if (processor->registers->reset == 0x001) {
        log_warn("Reset is asserted, not ticking clock");
        return;
    }

    for (uint32_t i = 0; i < num_cycles; i++) {
        enum processor_status tick_status = processor_tick(processor);  // MARK: add breakpoint detection
        if (tick_status != PROCESSOR_STATUS_SUCCESS) {
            log_warn("Execution stopped before requested number of cycles (errno %d)", tick_status);
            return;
        }
    }
}


static void cli_process_unbreak(struct processor *processor, int argc, char **argv) {
    (void) processor;
    (void) argc;
    (void) argv;
}


static void cli_process_verbose(struct processor *processor, int argc, char **argv) {
    (void) processor;

    switch (argc) {
    case 0:
        printf("Current debug level is: %d\n", logger_log_level);
        break;
    case 1:
        logger_set_level(atoi(argv[0]));
        break;
    default:
        log_error("Unexpected arguments");
        return;
    }
}


static void cli_extract_arguments(char *line, int *argc, char **argv) {
    *argc = 0;
    char *token = strtok(line, " \t\r\n");
    while (token != NULL) {
        if (*argc >= CLI_MAX_COMMAND_ARGUMENTS) {
            log_error("Too many command arguments provided");
            *argc = 0;
            return;
        }

        argv[(*argc)++] = token;
        token = strtok(NULL, " \t\r\n");
    }
}


void cli_run(struct processor *processor) {
    while (true) {
        printf("(sim) ");
        fflush(stdout);

        char command[CLI_MAX_COMMAND_LENGTH];
        char *result = fgets(command, CLI_MAX_COMMAND_LENGTH, stdin);
        if (result == NULL) {
            log_fatal("Could not read command from standard input");
        }

        size_t num_commands = sizeof(cli_command_table) / sizeof(cli_command_table[0]);
        for (size_t i = 0; i < num_commands; i++) {
            struct cli_command_descriptor descriptor = cli_command_table[i];
            char *command_name = strstr(command, descriptor.name);
            if (command_name != NULL) {
                int argc;
                char *argv[CLI_MAX_COMMAND_ARGUMENTS];
                cli_extract_arguments(command + strlen(descriptor.name), &argc, argv);
                descriptor.handler(processor, argc, argv);
                break;
            }
        }
    }
}
