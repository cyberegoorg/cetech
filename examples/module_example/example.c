#include <cetech/core/api/api_system.h>
#include <cetech/core/log/log.h>
#include <cetech/core/module/module.h>
#include <celib/macros.h>

#include "cetech/core/config/config.h"
#include "cetech/engine/input/input.h"

CETECH_DECL_API(ct_keyboard_a0)

CETECH_DECL_API(ct_log_a0)

void update() {
//    if (ce_keyboard_api.button_state(0, ce_keyboard_api.button_index("v"))) {
//        ce_log_api.warning("example", "V");
//    }
}

CETECH_MODULE_DEF(
        example,
        {
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_log_a0);
        },
        {
            CEL_UNUSED(api);
            ct_log_a0.info("example", "Init %d", reload);
        },
        {
            CEL_UNUSED(api);
            ct_log_a0.info("example", "Shutdown %d", reload);
        }
)
