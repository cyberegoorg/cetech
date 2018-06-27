#include <stdio.h>

#include "corelib/macros.h"

#include <corelib/api_system.h>
#include <corelib/os.h>
#include <corelib/module.h>
#include "corelib/log.h"


int exec(const char *argv) {
#if defined(CETECH_LINUX) || defined(CETECH_DARWIN)
    char output[4096];

    ct_log_a0->debug("os_sdl", "exec %s", argv);

    FILE *fp = popen(argv, "r");
    if (fp == NULL)
        return -1;

    while (fgets(output, CT_ARRAY_LEN(output), fp) != NULL) {
        printf("%s", output);
    }

    int status = pclose(fp);

    if (status == -1) {
        ct_log_a0->error("os_sdl", "output %s", output);
    }

    return status;
#else
#error "Implement this"
#endif
}

struct ct_process_a0 process_api = {
        .exec = exec
};

struct ct_process_a0 *ct_process_a0 = &process_api;
