#pragma once

#include <cstring>

namespace cetech {
    namespace command_line_globals {
        /*! Set argument for command line parser.
         * \param argc Argument count.
         * \param argv Argument strings.
         */
        void set_args(int argc, const char** argv);

        /*! Find argument.
         * \param longopt Long option.
         * \param shortopt Shortopt option.
         * \return Argument index or ARGC if argument not found.
         */
        int find_argument(const char* longopt, char shortopt = '\0');

        /*! Get argument parameter.
         * \param longopt Long option.
         * \param shortopt Shortopt option.
         * \return Parameter or NULL.
         */
        const char* get_parameter(const char* longopt, char shortopt = '\0');

        /*! Has command line argument.
         * \param longopt Long option.
         * \param shortopt Shortopt option.
         * \return True if has else false.
         */
        bool has_argument(const char* longopt, char shortopt = '\0');
    }
}