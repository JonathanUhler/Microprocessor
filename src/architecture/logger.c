#include "architecture/logger.h"


enum logger_log_level logger_log_level;


void logger_set_level(enum logger_log_level new_level) {
    logger_log_level = new_level;
}
