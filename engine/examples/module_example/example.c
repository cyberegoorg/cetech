#include <stdio.h>
#include <cetech/core/api.h>
#include <cetech/core/log.h>

#include "cetech/core/config.h"
#include "cetech/core/module.h"
#include "cetech/modules/input.h"

IMPORT_API(keyboard_api_v0)
IMPORT_API(log_api_v0)


void _init_api(struct api_v0 *api) {
    GET_API(api, keyboard_api_v0);
    GET_API(api, log_api_v0);
}

void init(struct api_v0 *api) {
    _init_api(api);

    log_api_v0.info("example", "Init");

}

void _shutdown() {
    log_api_v0.info("example", "Shutdown");
}

void *_reload_begin(struct api_v0 *api_v0) {
    return NULL;
}

void _update() {
    if (keyboard_api_v0.button_state(0, keyboard_api_v0.button_index("v"))) {
        log_api_v0.warning("example", "V");
    }
}

void _reload_end(struct api_v0 *api,
                 void *data) {
    _init_api(api);
    init(api);
}

void *get_module_api(int api) {
    if (api == PLUGIN_EXPORT_API_ID) {
        static struct module_api_v0 module = {0};

        module.init = init;
        module.shutdown = _shutdown;
        module.reload_begin = _reload_begin;
        module.reload_end = _reload_end;
        module.update = _update;

        return &module;
    }

    return NULL;
}
