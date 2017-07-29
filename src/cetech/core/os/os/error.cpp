
#include <string.h>

#if defined(CETECH_LINUX)

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/memory/private/allocator_core_private.h>
#include <cetech/core/log/log.h>
#include <cetech/core/os/errors.h>
#include <cetech/core/module/module.h>
#include "celib/allocator.h"

#endif

CETECH_DECL_API(ct_log_a0);

char *stacktrace(int skip) {
    auto *a = core_allocator::get();

#if defined(CETECH_LINUX)
    char *return_str = CEL_ALLOCATE(a, char, 4096);
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

    CEL_FREE(a, messages);
    return return_str;
#endif
}

void stacktrace_free(char *st) {
    auto *a = core_allocator::get();
    CEL_FREE(a, st);
}


void ct_error_assert(const char *where,
                     const char *condition,
                     const char *filename,
                     int line) {
    char *st = stacktrace(1);
    ct_log_a0.error(where,
                    "msg: \"%s\n  file: %s:%d\n  stacktrace:\n%s",
                    condition,
                    filename,
                    line,
                    st);
    stacktrace_free(st);
    abort();
}

static ct_error_a0 error_api = {
        .assert = ct_error_assert
};

CETECH_MODULE_DEF(
        error,
        {
            CETECH_GET_API(api, ct_log_a0);
        },
        {
            api->register_api("ct_error_a0", &error_api);
        },
        {
            CEL_UNUSED(api);
        }
)
