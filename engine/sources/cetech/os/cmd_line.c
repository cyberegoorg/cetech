#include <stddef.h>

#include <cetech/allocator.h>
#include <cetech/errors.h>
#include <cetech/log.h>
#include <cetech/string.h>
#include <cetech/cmd_line.h>


static int _cmd_line_is_short(struct args args,
                                  int i,
                                  char shortopt) {
    return (shortopt != '\0' &&
            str_len(args.argv[i]) > 1 &&
            args.argv[i][0] == '-' &&
            args.argv[i][1] == shortopt);
}

static int _cmd_line_is_long(struct args args,
                                 const int i,
                                 const char *longopt) {
    return (str_len(args.argv[i]) > 2 &&
            args.argv[i][0] == '-' &&
            args.argv[i][1] == '-' &&
            str_cmp(&args.argv[i][2], longopt) == 0);
}

int cmd_find_argument(struct args args,
                          const char *longopt,
                          char shortopt) {

    for (int i = 0; i < args.argc; i++) {
        if (_cmd_line_is_short(args, i, shortopt) ||
            _cmd_line_is_long(args, i, longopt)) {
            return i;
        }
    }

    return args.argc;
}

const char *cmd_get_parameter(struct args args,
                                  const char *longopt,
                                  char shortopt,
                                  int param) {

    int idx = cmd_find_argument(args, longopt, shortopt);

    if (idx < args.argc) {
        return args.argv[idx + 1 + param];
    }

    return NULL;
}

int cmd_has_argument(struct args args,
                         const char *longopt,
                         char shortopt) {
    return cmd_find_argument(args, longopt, shortopt) < args.argc;
}