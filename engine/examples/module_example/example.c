#include <cetech/kernel/api_system.h>
#include <cetech/kernel/log.h>

#include "cetech/kernel/config.h"
#include "cetech/kernel/module.h"
#include "cetech/modules/input.h"

CETECH_DECL_API(ct_keyboard_api_v0)
CETECH_DECL_API(ct_log_api_v0)

void update() {
//    if (ce_keyboard_api_v0.button_state(0, ce_keyboard_api_v0.button_index("v"))) {
//        ce_log_api_v0.warning("example", "V");
//    }
}


void load_module(struct ct_api_v0 *api) {
    CETECH_GET_API(api, ct_keyboard_api_v0);
    CETECH_GET_API(api, ct_log_api_v0);

    ct_log_api_v0.info("example", "Init");
}

void unload_module(struct ct_api_v0 *api) {
    ct_log_api_v0.info("example", "Shutdown");
}