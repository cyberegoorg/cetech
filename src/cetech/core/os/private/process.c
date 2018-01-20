#include <stdio.h>

#include "cetech/core/macros.h"

#include <cetech/core/api/api_system.h>
#include <cetech/core/os/process.h>
#include <cetech/core/module/module.h>
#include "cetech/core/log/log.h"

CETECH_DECL_API(ct_log_a0);

int exec(const char *argv) {
#if defined(CETECH_LINUX) || defined(CETECH_DARWIN)
    char output[4096];

    ct_log_a0.debug("os_sdl", "exec %s", argv);

    FILE *fp = popen(argv, "r");
    if (fp == NULL)
        return -1;

    while (fgets(output, CETECH_ARRAY_LEN(output), fp) != NULL) {
        printf("%s", output);
    }

    int status = pclose(fp);

    if (status == -1) {
        ct_log_a0.error("os_sdl", "output %s", output);
    }

    return status;
#else
#error "Implement this"
#endif
}

static struct ct_process_a0 process_api = {
        .exec = exec
};


CETECH_MODULE_DEF(
        process,
        {
            CETECH_GET_API(api, ct_log_a0);

        },
        {
            CT_UNUSED(reload);
            api->register_api("ct_process_a0", &process_api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
        }
)
