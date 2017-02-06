#ifndef CELIB_STACKTRACE_H
#define CELIB_STACKTRACE_H

#include <string.h>

#include "../memory/allocator.h"

#if defined(CELIB_LINUX)

#include <execinfo.h>

#endif

static char *cel_stacktrace(int skip) {
#if defined(CELIB_LINUX)
    char *return_str = (char *) cel_malloc(4096 * 8);
    return_str[0] = '\0';

    void *array[50];
    int size = backtrace(array, 50);

    char **messages = backtrace_symbols(array, size);

    char buffer[4096];
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

        if (mangled_name && offset_begin && offset_end && mangled_name < offset_begin) {
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

    cel_free(messages);
    return return_str;
#endif
}

static void cel_stacktrace_free(char *st) {
    cel_free(st);
}

#endif //CELIB_STACKTRACE_H
