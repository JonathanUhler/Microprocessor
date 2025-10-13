#ifndef _ASSEMBLER_LOGGER_H_
#define _ASSEMBLER_LOGGER_H_


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


#define LOGGER_COLOR_RESET  "\033[0m"
#define LOGGER_COLOR_DEBUG "\033[90m"
#define LOGGER_COLOR_INFO  "\033[32m"
#define LOGGER_COLOR_WARN  "\033[33m"
#define LOGGER_COLOR_ERROR "\033[91m\033[1m"
#define LOGGER_COLOR_FATAL "\033[38;5;124m\033[1m"


#define LOG_PRINT_HELPER(severity, format, ...)                 \
    fprintf(stderr, "%-5s: " format "\n%s", severity, __VA_ARGS__)

#define LOG_PRINT(severity, ...)                \
    LOG_PRINT_HELPER(severity, __VA_ARGS__, "")

#define log_error(...)                                                  \
    {                                                                   \
        LOG_PRINT(LOGGER_COLOR_ERROR "error" LOGGER_COLOR_RESET, __VA_ARGS__); \
    }

#define log_fatal(...)                                                  \
    {                                                                   \
        LOG_PRINT(LOGGER_COLOR_FATAL "fatal" LOGGER_COLOR_RESET, __VA_ARGS__);        \
        exit(1);                                                        \
    }


#endif  // _LOGGER_H_
