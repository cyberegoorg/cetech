#ifndef CETECH_CMD_LINE_H
#define CETECH_CMD_LINE_H

#include <celib/string/string.h>

struct args {
    int argc;
    const char **argv;
};

static inline int _os_cmd_line_is_short(struct args args,
                                        int i,
                                        char shortopt) {
    return (shortopt != '\0' &&
            str_lenght(args.argv[i]) > 1 &&
            args.argv[i][0] == '-' &&
            args.argv[i][1] == shortopt);
}

static inline int _os_cmd_line_is_long(struct args args,
                                       const int i,
                                       const char *longopt) {
    return (str_lenght(args.argv[i]) > 2 &&
            args.argv[i][0] == '-' &&
            args.argv[i][1] == '-' &&
            str_compare(&args.argv[i][2], longopt) == 0);
}

static int os_cmd_find_argument(struct args args,
                                const char *longopt,
                                char shortopt) {

    for (int i = 0; i < args.argc; i++) {
        if (_os_cmd_line_is_short(args, i, shortopt) || _os_cmd_line_is_long(args, i, longopt)) {
            return i;
        }
    }

    return args.argc;
}

static const char *os_cmd_get_parameter(struct args args,
                                        const char *longopt,
                                        char shortopt,
                                        int param) {

    int idx = os_cmd_find_argument(args, longopt, shortopt);

    if (idx < args.argc) {
        return args.argv[idx + 1 + param];
    }

    return NULL;
}

static int os_cmd_has_argument(struct args args,
                               const char *longopt,
                               char shortopt) {
    return os_cmd_find_argument(args, longopt, shortopt) < args.argc;
}

#endif //CETECH_CMD_LINE_H
