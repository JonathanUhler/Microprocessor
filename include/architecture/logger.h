/**
 * Basic logging facilities for the software collection.
 *
 * @author Jonathan Uhler
 */


#ifndef _ARCHITECTURE_LOGGER_H_
#define _ARCHITECTURE_LOGGER_H_


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


/** ANSI reset color code. */
#define LOGGER_COLOR_RESET "\033[0m"
/** ANSI color code for trace level messages (light gray). */
#define LOGGER_COLOR_TRACE "\033[37m"
/** ANSI color code for debug level messages (black). */
#define LOGGER_COLOR_DEBUG "\033[30m"
/** ANSI color code for info level messages (green). */
#define LOGGER_COLOR_INFO  "\033[32m"
/** ANSI color code for warning level messages (yellow). */
#define LOGGER_COLOR_WARN  "\033[33m"
/** ANSI color code for error level messages (bold, red). */
#define LOGGER_COLOR_ERROR "\033[91m\033[1m"
/** ANSI color code for fatal level messages (bold, dark red). */
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


/**
 * Possible logging level values.
 *
 * Values increment as the messages become more verbose. If a program has the ability to count the
 * occurances of a flag like -v, that can be used to set the log level.
 *
 * Fatal messages will always trigger, and by default (log level 0) no other messages are printed.
 */
enum logger_log_level {
    /** Allow all messages up to trace. */
    LOGGER_LEVEL_TRACE = 5,
    /** Allow all messages up to debug. */
    LOGGER_LEVEL_DEBUG = 4,
    /** Allow all messages up to info. */
    LOGGER_LEVEL_INFO  = 3,
    /** Allow all messages up to warning. */
    LOGGER_LEVEL_WARN  = 2,
    /** Allow only fatal and error messages. */
    LOGGER_LEVEL_ERROR = 1
};


/** The current logger level. */
extern enum logger_log_level logger_log_level;


/**
 * Sets a new logging level.
 *
 * @param new_level  The new logging level.
 */
void logger_set_level(enum logger_log_level new_level);


#endif  // _ARCHITECTURE_LOGGER_H_
