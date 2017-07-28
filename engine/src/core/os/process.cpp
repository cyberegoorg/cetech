#include <dlfcn.h>

#include <cetech/core/os/errors.h>
#include <cetech/core/api_system.h>
#include <cetech/core/log.h>
#include <cetech/machine/machine.h>
#include <stdio.h>
#include <cetech/core/log.h>
#include <cetech/core/os/process.h>
#include <celib/macros.h>

CETECH_DECL_API(ct_log_a0);

int exec(const char *argv) {
#if defined(CETECH_LINUX)
    char output[4096];

    ct_log_a0.debug("os_sdl", "exec %s", argv);

    FILE *fp = popen(argv, "r");
    if (fp == NULL)
        return 0;

    while (fgets(output, CETECH_ARRAY_LEN(output), fp) != NULL) {
        printf("%s", output);
    }

    int status = fclose(fp);

    if (status != 0) {
        ct_log_a0.error("os_sdl", "output %s", output);
    }

    return status;
#endif
}

static ct_process_a0 process_api = {
        .exec = exec
};

extern "C" void process_load_module(ct_api_a0 *api) {
    CETECH_GET_API(api, ct_log_a0);

    api->register_api("ct_process_a0", &process_api);
}

extern "C" void process_unload_module(ct_api_a0 *api) {
    CEL_UNUSED(api);
}