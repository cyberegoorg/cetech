/*******************************************************************************
**** Includes
*******************************************************************************/
#include <execinfo.h>
#include <cxxabi.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>


/*******************************************************************************
**** Implementation
*******************************************************************************/
namespace cetech {
    char* stacktrace(int skip) {
        char* return_str = (char*)malloc(4096 * 8);
        return_str[0] = '\0';

        void* array[50];
        int size = backtrace(array, 50);

        char** messages = backtrace_symbols(array, size);

        char buffer[4096];
        for (int i = skip; i < size && messages != NULL; ++i) {
            buffer[0] = '\0';
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

                sprintf(buffer,
                        "    - %s: (%s)+%s %s\n",
                        messages[i],
                        (status == 0 ? real_name : mangled_name),
                        offset_begin,
                        offset_end);
                strcat(return_str, buffer);
                free(real_name);

            } else {
                sprintf(buffer, "    - %s\n", messages[i]);
                strcat(return_str, buffer);
            }
        }

        free(messages);

        return return_str;
    }
}
