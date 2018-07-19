#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <corelib/allocator.h>

#include <corelib/api_system.h>
#include <corelib/log.h>
#include <corelib/os.h>
#include <corelib/module.h>

#include <corelib/platform.h>

#if CT_PLATFORM_LINUX || CT_PLATFORM_OSX
#include <execinfo.h>
#endif

#include <corelib/macros.h>
#include <corelib/memory.h>

char *stacktrace(int skip) {
#if CT_PLATFORM_LINUX || CT_PLATFORM_OSX
    struct ct_alloc *a = ct_memory_a0->system;

    char *return_str = CT_ALLOC(a, char, 4096);
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

    CT_FREE(a, messages);
    return return_str;
#endif

    return NULL;
}

void stacktrace_free(char *st) {
    struct ct_alloc *a = ct_memory_a0->system;

    CT_FREE(a, st);
}


void ct_error_assert(const char *where,
                     const char *condition,
                     const char *filename,
                     int line) {
    char *st = stacktrace(1);
    ct_log_a0->error(where,
                     "msg: \"%s\n  file: %s:%d\n  stacktrace:\n%s",
                     condition,
                     filename,
                     line,
                     st);
    stacktrace_free(st);
    abort();
}

struct ct_os_error_a0 error_api = {
        .assert = ct_error_assert
};

struct ct_os_error_a0 *ct_error_a0 = &error_api;
