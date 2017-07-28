#include <cetech/core/api_system.h>
#include <cetech/core/log.h>

#include "cetech/core/config.h"
#include "cetech/engine/input.h"

CETECH_DECL_API(ct_keyboard_a0)

CETECH_DECL_API(ct_log_a0)

void update() {
//    if (ce_keyboard_api.button_state(0, ce_keyboard_api.button_index("v"))) {
//        ce_log_api.warning("example", "V");
//    }
}


void load_module(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_keyboard_a0);
    CETECH_GET_API(api, ct_log_a0);

    ct_log_a0.info("example", "Init");
}

void unload_module(struct ct_api_a0 *api) {
    ct_log_a0.info("example", "Shutdown");
}