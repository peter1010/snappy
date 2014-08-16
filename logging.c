#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "logging.h"

static unsigned log_level = LOG_WARN_LVL;
static bool log_append_allowed = false;
static FILE * log_out = NULL;

/**
 * Initialiser the logger by opening the output stream
 */
void log_init()
{
    if(!log_out)
    {
        log_out = stderr;
    }
}


/**
 * Set logging level
 *
 * @param[in] level, the level to set
 */
void set_logging_level(unsigned level)
{
    log_level = level;
}


/**
 * Open the logger, called before each log call, it also tests if we want to
 * log. If logging, then the initial message prefix is sent.
 *
 * @param[in] category (usually the source file name)
 * @param[in] level
 * 
 * @return True if log, false if not
 */
bool open_logger(const char * category, unsigned level)
{
    if(level <= log_level)
    {
        log_init();
        fprintf(log_out, "[%s", category);
        return true;
    }
    else
    {
        log_append_allowed = false;
        return false;
    }
}


/**
 * Log a message
 *
 * @param[in] filename Name of file that log was called from
 * @param[in] line Line number where log called from
 * @param[in] level The level
 * @param[in] fmt The format string in the style of printf
 * @param[in] args Variable args
 */
void log_msg(int line, const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    fprintf(log_out, ":%i] ", line);
    vfprintf(log_out, fmt, ap);
    fputs("\n", log_out);

    va_end(ap);
    log_append_allowed = true;
}

void log_errno(int line, const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    fprintf(log_out, ":%i] ", line);
    vfprintf(log_out, fmt, ap);
    fputs(":", log_out);
    fputs(strerror(errno), log_out);
    fputs("\n", log_out);

    va_end(ap);
    log_append_allowed = true;
}

void log_msg_append(const char * fmt, ...)
{
    if(log_append_allowed)
    {
        va_list ap;
        va_start(ap, fmt);

        fputs("\t", log_out);
        vfprintf(log_out, fmt, ap);
        fputs("\n", log_out);

        va_end(ap);
    }
}
