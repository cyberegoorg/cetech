#include <cetech/kernel/api.h>
#include <cetech/kernel/log.h>

#include "cetech/kernel/config.h"
#include "cetech/kernel/module.h"
#include "cetech/modules/input.h"

CETECH_DECL_API(keyboard_api_v0)
CETECH_DECL_API(log_api_v0)

void update() {
//    if (keyboard_api_v0.button_state(0, keyboard_api_v0.button_index("v"))) {
//        log_api_v0.warning("example", "V");
//    }
}


void load_module(struct api_v0 *api) {
    CETECH_GET_API(api, keyboard_api_v0);
    CETECH_GET_API(api, log_api_v0);

    log_api_v0.info("example", "Init");
}

void unload_module(struct api_v0 *api) {
    log_api_v0.info("example", "Shutdown");
}