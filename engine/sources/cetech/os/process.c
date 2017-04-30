
#include <stdio.h>

#include "cetech/log.h"

int exec(const char *argv) {
#if defined(CELIB_LINUX)
    char output[4096];

    log_debug("os", "exec %s", argv);

    FILE *fp = popen(argv, "r");
    if (fp == NULL)
        return 0;

    while (fgets(output, CEL_ARRAY_LEN(output), fp) != NULL) {
        printf("%s", output);
    }

    int status = fclose(fp);

    if (status != 0) {
        log_error("os", "output %s", output);
    }

    return status;
#endif
}