#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <celib/memory/allocator.h>

#include <celib/api.h>
#include <celib/log.h>

#include <celib/module.h>

#include <celib/platform.h>

#if CE_PLATFORM_LINUX || CE_PLATFORM_OSX

#include <execinfo.h>

#endif

#include <celib/macros.h>
#include <celib/memory/memory.h>

char *stacktrace(int skip) {
#if CE_PLATFORM_LINUX || CE_PLATFORM_OSX
    ce_alloc_t0 *a = ce_memory_a0->system;

    char *return_str = CE_ALLOC(a, char, 4096);
    return_str[0] = '\0';

    void *array[32];
    int size = backtrace(array, 32);

    char **messages = backtrace_symbols(array, size);

    char buffer[256];
    for (int i = skip; i < size && messages != NULL; ++i) {
        buffer[0] = '\0';
        char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;

        for (char *p = messages[i]; *p; ++p) {
            if (*p == '(') {
                mangled_name = p;
            } else if (*p == '+') {
                offset_begin = p;
            } else if (*p == ')') {
                offset_end = p;
                break;
            }
        }

        if (mangled_name && offset_begin && offset_end &&
            mangled_name < offset_begin) {
            *mangled_name++ = '\0';
            *offset_begin++ = '\0';
            *offset_end++ = '\0';

            sprintf(buffer,
                    "    - %s: %s() +%s %s\n",
                    messages[i],
                    mangled_name,
                    offset_begin,
                    offset_end);

            strcat(return_str, buffer);

        } else {
            sprintf(buffer, "    - %s\n", messages[i]);
            strcat(return_str, buffer);
        }
    }

    CE_FREE(a, messages);
    return return_str;
#endif

    return NULL;
}

void stacktrace_free(char *st) {
    ce_alloc_t0 *a = ce_memory_a0->system;

    CE_FREE(a, st);
}


void ct_error_assert(const char *where,
                     const char *condition,
                     const char *filename,
                     int line) {

    ce_log_a0->error(where,
                     "assert: %s %s:%d",
                     condition,
                     filename,
                     line);
    abort();
}

struct ce_os_error_a0 error_api = {
        .assert = ct_error_assert,
        .stacktrace = stacktrace,
        .stacktrace_free = stacktrace_free,

};

struct ce_os_error_a0 *ce_os_error_a0 = &error_api;
