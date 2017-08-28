#include "celib/map.inl"

#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/playground/property_inspector.h>

#include "cetech/kernel/hashlib.h"
#include "cetech/kernel/config.h"
#include "cetech/kernel/memory.h"
#include "cetech/kernel/api_system.h"
#include "cetech/kernel/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);

using namespace celib;

#define _G property_inspector_global
static struct _G {
    ct_pi_on_debugui on_debugui;
    bool visible;
} _G;


static void set_active(ct_pi_on_debugui on_debugui) {
    _G.on_debugui = on_debugui;
}

static ct_property_inspector_a0 property_inspector_api = {
        .set_active = set_active
};


static void on_debugui() {
    if (ct_debugui_a0.BeginDock("Property inspector", &_G.visible,
                                DebugUIWindowFlags_(0))) {
        if (_G.on_debugui) {
            _G.on_debugui();
        }
    }
    ct_debugui_a0.EndDock();
}

static void _init(ct_api_a0 *api) {
    _G = {
            .visible = true
    };

    api->register_api("ct_property_inspector_a0", &property_inspector_api);
    ct_debugui_a0.register_on_debugui(on_debugui);

}

static void _shutdown() {
    _G = {};
}

CETECH_MODULE_DEF(
        property_inspector,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_debugui_a0);
        },
        {
            _init(api);
        },
        {
            CEL_UNUSED(api);
            _shutdown();
        }
)