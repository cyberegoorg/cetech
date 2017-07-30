#include <celib/macros.h>

#include <cetech/core/api/api_system.h>
#include <cetech/core/log/log.h>
#include <cetech/core/module/module.h>
#include <cetech/core/config/config.h>
#include <cetech/engine/input/input.h>
#include <cetech/engine/application/application.h>

CETECH_DECL_API(ct_log_a0)
CETECH_DECL_API(ct_app_a0)
CETECH_DECL_API(ct_keyboard_a0)

void update(float dt) {
    if (ct_keyboard_a0.button_state(0, ct_keyboard_a0.button_index("v"))) {
        ct_log_a0.warning("example", "VVVVVsssddddddd    sVVVVsssV");
    }
}

CETECH_MODULE_DEF(
        example,
        {
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_app_a0);
        },
        {
            CEL_UNUSED(api);
            ct_log_a0.info("example", "Init %d", reload);
            ct_app_a0.register_on_update(update);
        },
        {
            CEL_UNUSED(api);

            ct_log_a0.info("example", "Shutdown %d", reload);
            ct_app_a0.unregister_on_update(update);
        }
)
