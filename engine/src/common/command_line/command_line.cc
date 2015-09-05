#include <cstring>

namespace cetech {
    namespace internal {
        static int _argc;
        static const char** _argv;
    }

    namespace command_line {
        void set_args(int argc, const char** argv) {
            internal::_argc = argc;
            internal::_argv = argv;
        }

        int find_argument(const char* longopt, char shortopt) {
            for (int i = 0; i < internal::_argc; i++) {
                if ((shortopt != '\0' && strlen(internal::_argv[i]) > 1 && internal::_argv[i][0] == '-' &&
                     internal::_argv[i][1] == shortopt) ||
                    (strlen(internal::_argv[i]) > 2 && internal::_argv[i][0] == '-' && internal::_argv[i][1] == '-' &&
                     strcmp(&internal::_argv[i][2], longopt) == 0)) {
                    return i;
                }
            }

            return internal::_argc;
        }

        const char* get_parameter(const char* longopt, char shortopt) {
            int idx = find_argument(longopt, shortopt);
            return idx < internal::_argc ? internal::_argv[idx + 1] : nullptr;
        }

        bool has_argument(const char* longopt, char shortopt) {
            return find_argument(longopt, shortopt) < internal::_argc;
        }


    }
}