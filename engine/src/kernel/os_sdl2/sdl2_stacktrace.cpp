
#include <string.h>

#if defined(CETECH_LINUX)

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

#include <cetech/kernel/api_system.h>
#include <cetech/kernel/errors.h>
#include <cetech/kernel/log.h>

#include "../memory/memory_private.h"

#include "../memory/core_allocator_private.h"

#endif

char *stacktrace(int skip) {
    auto* a = core_allocator::get();

#if defined(CETECH_LINUX)
    char *return_str = CETECH_ALLOCATE(a, char, 2048);
    return_str[0] = '\0';

    void *array[50];
    int size = backtrace(array, 50);

    char **messages = backtrace_symbols(array, size);

    char buffer[1024];
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

    CETECH_FREE(a, messages);
    return return_str;
#endif
}

void stacktrace_free(char *st) {
    auto* a = core_allocator::get();
    CETECH_FREE(a, st);
}

CETECH_DECL_API(log_api_v0);

void error_assert(const char *where,
                  const char *condition,
                  const char *filename,
                  int line) {
    char *st = stacktrace(1);
    log_api_v0.error(where,
                     "msg: \"%s\n  file: %s:%d\n  stacktrace:\n%s",
                     condition,
                     filename,
                     line,
                     st);
    stacktrace_free(st);
    abort();
}

static struct error_api_v0 error_api = {
        .assert = error_assert
};

void error_register_api(struct api_v0 *api) {
    CETECH_GET_API(api, log_api_v0);

    api->register_api("error_api_v0", &error_api);
}
