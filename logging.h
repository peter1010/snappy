#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdbool.h>

/**
 * Some Logging code
 */

#define LOG_TIMESTAMP

/**
 * Logging levels
 */
enum Log_Level_enum
{
    LOG_ERROR_LVL,
    LOG_WARN_LVL,
    LOG_INFO_LVL,
    LOG_DEBUG_LVL
};

extern void set_logging_level(unsigned level);


#ifdef __BASE_FILE__
static const char log_category[] = __BASE_FILE__;
#define SET_LOG_CATEGORY
#else
/* Place this macro at top of all files that using logging */
#define SET_LOG_CATEGORY static const char log_category[] = __FILE__
#endif


#define _LOG_MSG(cat, level, ...) \
    do { void * hnd = open_logger(cat, level); \
         if(hnd) \
            log_msg(hnd, __LINE__, __VA_ARGS__); \
    } while(0)

#define _LOG_ERRNO(cat, level, ...)\
    do { void * hnd = open_logger(cat, level); \
         if(hnd) \
            log_errno(hnd, __LINE__, __VA_ARGS__); \
    } while(0)


/* Use these macros to log messages for different levels */
#define LOG_ERROR(...) _LOG_MSG(log_category, LOG_ERROR_LVL, __VA_ARGS__)
#define LOG_WARN(...)  _LOG_MSG(log_category, LOG_WARN_LVL,  __VA_ARGS__)
#define LOG_INFO(...)  _LOG_MSG(log_category, LOG_INFO_LVL,  __VA_ARGS__)
#define LOG_DEBUG(...) _LOG_MSG(log_category, LOG_DEBUG_LVL, __VA_ARGS__)

#define LOG_ERRNO_AS_ERROR(...) _LOG_ERRNO(log_category, LOG_ERROR_LVL, __VA_ARGS__)

extern void * open_logger(const char * category, unsigned level);

extern void log_msg(
    void * hnd,
    int line,
    const char * fmt,
    ...) __attribute__((format (printf, 3, 4)));


extern void log_errno(
    void * hnd,
    int line,
    const char * fmt,
    ...) __attribute__((format (printf, 3, 4)));

#endif
