#include <stdio.h>
#include <string.h>

#include <execinfo.h>
#include <celib/os/memory.h>

#include "celib/memory/memory.h"


char *utils_stacktrace(int skip) {
#if defined(CETECH_LINUX)
    char *return_str = (char *) os_malloc(4096 * 8);
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

    os_free(messages);
    return return_str;
#endif
    return NULL;
}

void utils_stacktrace_free(char *st) {
    os_free(st);
}