/***********************************************************************
**** Includes
***********************************************************************/

#include <stdio.h>

#if defined(CETECH_LINUX)

#include <sys/file.h>

#endif

#include "celib/log/log.h"
#include "celib/string/string.h"

/***********************************************************************
**** Internals
***********************************************************************/

#define COLOR_RED  "\x1B[31m"
#define COLOR_GREEN  "\x1B[32m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_BLUE  "\x1B[34m"
#define COLOR_RESET "\033[0m"

#define LOG_FORMAT   \
    "---\n"          \
    "level: %s\n"    \
    "where: %s\n"    \
    "time: %s\n"     \
    "worker: %d\n"   \
    "msg: |\n  %s\n" \


#ifdef CETECH_COLORED_LOG
#define COLORED_TEXT(color, text) color text COLOR_RESET
#else
#define COLORED_TEXT(color, text) text
#endif

static const char *_level_to_str[4] = {
        [LOG_INFO]    = "info",
        [LOG_WARNING] = "warning",
        [LOG_ERROR]   = "error",
        [LOG_DBG]     = "debug"
};

static const char *_level_format[4] = {
        [LOG_INFO]    = COLORED_TEXT(COLOR_BLUE, LOG_FORMAT),
        [LOG_WARNING] = COLORED_TEXT(COLOR_YELLOW, LOG_FORMAT),
        [LOG_ERROR]   = COLORED_TEXT(COLOR_RED, LOG_FORMAT),
        [LOG_DBG]     = COLORED_TEXT(COLOR_GREEN, LOG_FORMAT)
};

static const char *_nocolor_level_format[4] = {
        [LOG_INFO]    = LOG_FORMAT,
        [LOG_WARNING] = LOG_FORMAT,
        [LOG_ERROR]   = LOG_FORMAT,
        [LOG_DBG]     = LOG_FORMAT
};


static char *_time_to_str(struct tm *gmtm) {
    char *time_str = asctime(gmtm);
    time_str[str_lenght(time_str) - 1] = '\0';
    return time_str;
}


/***********************************************************************
**** Interface implementation
***********************************************************************/

void log_stdout_handler(enum log_level level,
                        time_t time,
                        char worker_id,
                        const char *where,
                        const char *msg,
                        void *data) {

    FILE *out = level == LOG_ERROR ? stderr : stdout;

    struct tm *gmtm = gmtime(&time);
    const char *time_str = _time_to_str(gmtm);

#if defined(CETECH_LINUX)
    flock(out->_fileno, LOCK_EX);
#endif

    fprintf(out, _level_format[level], _level_to_str[level],
            where, time_str, worker_id, msg);

    fflush(out);

#if defined(CETECH_LINUX)
    flock(out->_fileno, LOCK_UN);
#endif
}
