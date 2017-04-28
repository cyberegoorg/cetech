#include <celib/string.h>
#include "celib/cmd_line.h"


static int _cel_cmd_line_is_short(struct args args,
                                  int i,
                                  char shortopt) {
    return (shortopt != '\0' &&
            cel_strlen(args.argv[i]) > 1 &&
            args.argv[i][0] == '-' &&
            args.argv[i][1] == shortopt);
}

static int _cel_cmd_line_is_long(struct args args,
                                 const int i,
                                 const char *longopt) {
    return (cel_strlen(args.argv[i]) > 2 &&
            args.argv[i][0] == '-' &&
            args.argv[i][1] == '-' &&
            cel_strcmp(&args.argv[i][2], longopt) == 0);
}

int cel_cmd_find_argument(struct args args,
                          const char *longopt,
                          char shortopt) {

    for (int i = 0; i < args.argc; i++) {
        if (_cel_cmd_line_is_short(args, i, shortopt) ||
            _cel_cmd_line_is_long(args, i, longopt)) {
            return i;
        }
    }

    return args.argc;
}

const char *cel_cmd_get_parameter(struct args args,
                                  const char *longopt,
                                  char shortopt,
                                  int param) {

    int idx = cel_cmd_find_argument(args, longopt, shortopt);

    if (idx < args.argc) {
        return args.argv[idx + 1 + param];
    }

    return NULL;
}

int cel_cmd_has_argument(struct args args,
                         const char *longopt,
                         char shortopt) {
    return cel_cmd_find_argument(args, longopt, shortopt) < args.argc;
}