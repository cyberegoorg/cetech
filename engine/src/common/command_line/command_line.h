#pragma once

#include <cstring>

namespace cetech {
    namespace command_line {
        void set_args(int argc, const char** argv);

        int find_argument(const char* longopt, char shortopt);
        const char* get_parameter(const char* longopt, char shortopt = '\0');
        bool has_argument(const char* longopt, char shortopt = '\0');
    }
}