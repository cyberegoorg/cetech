#include <stdio.h>

#include "celib/macros.h"

#include <celib/api_system.h>
#include <celib/os.h>
#include <celib/module.h>
#include "celib/log.h"


int exec(const char *argv) {
#if CE_PLATFORM_LINUX || CE_PLATFORM_OSX
    char output[4096];

    ce_log_a0->debug("os_sdl", "exec %s", argv);

    FILE *fp = popen(argv, "r");
    if (fp == NULL)
        return -1;

    while (fgets(output, CE_ARRAY_LEN(output), fp) != NULL) {
        printf("%s", output);
    }

    int status = pclose(fp);

    if (status == -1) {
        ce_log_a0->error("os_sdl", "output %s", output);
    }

    return status;
#else
#error "Implement this"
#endif
}

struct ce_os_process_a0 process_api = {
        .exec = exec
};

struct ce_os_process_a0 *ct_process_a0 = &process_api;
