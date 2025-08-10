#ifndef LOG_H
#define LOG_H

#define E_FAILSTRFTIME 9999

typedef enum
{
        LOG_LEVEL_INFO,
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_ERROR,
} log_level_t;

int iris_log(log_level_t level, const char *message, ...);

#endif

