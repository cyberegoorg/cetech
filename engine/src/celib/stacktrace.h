#pragma once

#include <execinfo.h>
#include <cxxabi.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace cetech {
    static char* stacktrace(int skip) {
        char* return_str = (char*)malloc(4096 * 8);
        return_str[0] = '\0';

        void* array[50];
        int size = backtrace(array, 50);

        char** messages = backtrace_symbols(array, size);

        for (int i = skip; i < size && messages != NULL; ++i) {
            char* mangled_name = 0, * offset_begin = 0, * offset_end = 0;

            for (char* p = messages[i]; *p; ++p) {
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

                int status;
                char* real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);

                if (status == 0) {
                    char buffer[4096] = {0};
                    sprintf(buffer,
                            "\t[%d] %s: (%s)+%s %s\n",
                            i,
                            messages[i],
                            (status == 0 ? real_name : mangled_name),
                            offset_begin,
                            offset_end);
                    strcat(return_str, buffer);
                    free(real_name);
                }
            } else {
                char buffer[4096] = {0};
                sprintf(buffer, "\t[%d] %s\n", i, messages[i]);
                strcat(return_str, buffer);
            }
        }

        free(messages);

        return return_str;
    }
}