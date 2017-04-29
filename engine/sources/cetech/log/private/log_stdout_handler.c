/***********************************************************************
**** Includes
***********************************************************************/

#include <stdio.h>

#if defined(CELIB_LINUX)

#include <sys/file.h>

#endif

#include "cetech/log/log.h"
#include "cetech/os/errors.h"

#include "cetech/memory/allocator.h"
#include "cetech/string/string.h"

/***********************************************************************
**** Internals
***********************************************************************/

#define FBLACK      "\033["

#define BRED        "31m"
#define BGREEN      "32m"
#define BYELLOW     "33m"
#define BBLUE       "34m"

#define NONE        "\033[0m"

#define LOG_FORMAT   \
    "---\n"          \
    "level: %s\n"    \
    "where: %s\n"    \
    "time: %s\n"     \
    "worker: %d\n"   \
    "msg: |\n  %s\n"


#ifdef CELIB_COLORED_LOG
#define COLORED_TEXT(color, text) FBLACK color text NONE
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
        [LOG_INFO]    = COLORED_TEXT(BBLUE, LOG_FORMAT),
        [LOG_WARNING] = COLORED_TEXT(BYELLOW, LOG_FORMAT),
        [LOG_ERROR]   = COLORED_TEXT(BRED, LOG_FORMAT),
        [LOG_DBG]     = COLORED_TEXT(BGREEN, LOG_FORMAT)
};

static const char *_nocolor_level_format[4] = {
        [LOG_INFO]    = LOG_FORMAT,
        [LOG_WARNING] = LOG_FORMAT,
        [LOG_ERROR]   = LOG_FORMAT,
        [LOG_DBG]     = LOG_FORMAT
};


static char *_time_to_str(struct tm *gmtm) {
    char *time_str = asctime(gmtm);
    time_str[cel_strlen(time_str) - 1] = '\0';
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

#if defined(CELIB_LINUX)
    flock(out->_fileno, LOCK_EX);
#endif

    fprintf(out, _level_format[level], _level_to_str[level],
            where, time_str, worker_id, msg);

    fflush_unlocked(out);

#if defined(CELIB_LINUX)
    flock(out->_fileno, LOCK_UN);
#endif
}
