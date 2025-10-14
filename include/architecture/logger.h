#ifndef _ARCHITECTURE_LOGGER_H_
#define _ARCHITECTURE_LOGGER_H_


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


#define LOGGER_COLOR_RESET "\033[0m"
#define LOGGER_COLOR_TRACE "\033[37m"
#define LOGGER_COLOR_DEBUG "\033[30m"
#define LOGGER_COLOR_INFO  "\033[32m"
#define LOGGER_COLOR_WARN  "\033[33m"
#define LOGGER_COLOR_ERROR "\033[91m\033[1m"
#define LOGGER_COLOR_FATAL "\033[38;5;124m\033[1m"


#define LOG_PRINT_HELPER(severity, format, ...)                 \
    fprintf(stderr, "%-5s: " format "\n%s", severity, __VA_ARGS__)

#define LOG_PRINT(severity, ...)                \
    LOG_PRINT_HELPER(severity, __VA_ARGS__, "")

#define log_trace(...)                                                  \
    if (logger_log_level >= LOGGER_LEVEL_TRACE) {                       \
        LOG_PRINT(LOGGER_COLOR_TRACE "trace" LOGGER_COLOR_RESET, __VA_ARGS__); \
    }

#define log_debug(...)                                                  \
    if (logger_log_level >= LOGGER_LEVEL_DEBUG) {                       \
        LOG_PRINT(LOGGER_COLOR_DEBUG "debug" LOGGER_COLOR_RESET, __VA_ARGS__); \
    }

#define log_info(...)                                                   \
    if (logger_log_level >= LOGGER_LEVEL_INFO) {                        \
        LOG_PRINT(LOGGER_COLOR_INFO "info" LOGGER_COLOR_RESET, __VA_ARGS__); \
    }

#define log_warn(...)                                                   \
    if (logger_log_level >= LOGGER_LEVEL_WARN) {                        \
        LOG_PRINT(LOGGER_COLOR_WARN "warn" LOGGER_COLOR_RESET, __VA_ARGS__); \
    }

#define log_error(...)                                                  \
    if (logger_log_level >= LOGGER_LEVEL_ERROR) {                       \
        LOG_PRINT(LOGGER_COLOR_ERROR "error" LOGGER_COLOR_RESET, __VA_ARGS__); \
    }

#define log_fatal(...)                                                  \
    {                                                                   \
        LOG_PRINT(LOGGER_COLOR_FATAL "fatal" LOGGER_COLOR_RESET, __VA_ARGS__); \
        exit(1);                                                        \
    }


enum logger_log_level {
    LOGGER_LEVEL_TRACE = 3,
    LOGGER_LEVEL_DEBUG = 2,
    LOGGER_LEVEL_INFO  = 1,
    LOGGER_LEVEL_WARN  = 0,
    LOGGER_LEVEL_ERROR = 0
};


extern enum logger_log_level logger_log_level;


void logger_set_level(enum logger_log_level new_level);


#endif  // _ARCHITECTURE_LOGGER_H_
