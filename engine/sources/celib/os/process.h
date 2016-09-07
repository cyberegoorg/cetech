#ifndef CETECH_PROCESS_H
#define CETECH_PROCESS_H

#include <celib/log/log.h>
#include "stdint.h"
#include "stdio.h"
#include "celib/types.h"

#if CETECH_LINUX
#endif

static int os_exec(const char *argv) {
    char output[4096];

    FILE *fp = popen(argv, "r");
    if (fp == NULL)
        return 0;

    while (fgets(output, CE_ARRAY_LEN(output), fp) != NULL) {
        printf("%s", output);
    }

    log_debug("os", "exec %s", argv);

    return fclose(fp);
}

#endif //CETECH_PROCESS_H
