#include <cetech/debugui/debugui.h>
#include <cetech/playground/property_editor.h>
#include <cetech/playground/asset_property.h>
#include <cetech/resource/resource.h>
#include <cetech/playground/asset_browser.h>

#include <corelib/hash.inl>
#include <corelib/ebus.h>
#include <corelib/ydb.h>

#include "corelib/hashlib.h"
#include "corelib/config.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/module.h"

#define _G asset_property_global
static struct _G {
    struct ct_hash_t on_asset_map;
    ct_ap_on_asset *on_asset;

    ct_ap_on_asset active_on_asset;

    struct ct_resource_id active_asset;

    const char *active_path;
    struct ct_alloc *allocator;
} _G;

static void on_debugui() {
    char filename[512] = {};
    ct_resource_a0->compiler_get_filename(filename,
                                          CT_ARRAY_LEN(filename),
                                          _G.active_asset);

    if (ct_debugui_a0->Button("Save", (float[2]) {0.0f})) {
        ct_ydb_a0->save(filename);
    }
    ct_debugui_a0->SameLine(0.0f, -1.0f);

    ct_debugui_a0->InputText("asset",
                             filename, strlen(filename),
                             DebugInputTextFlags_ReadOnly, 0, NULL);

    if (_G.active_on_asset) {
        _G.active_on_asset(_G.active_asset, _G.active_path);
    }
}

static void register_asset(uint32_t type,
                           ct_ap_on_asset on_asset) {
    ct_array_push(_G.on_asset, on_asset, _G.allocator);
    ct_hash_add(&_G.on_asset_map, type, ct_array_size(_G.on_asset) - 1,
                _G.allocator);
}

static void set_asset(uint64_t event) {

    uint64_t asset = ct_cdb_a0->read_uint64(event, CT_ID64_0("asset"), 0);
    const char *path = ct_cdb_a0->read_str(event, CT_ID64_0("path"), 0);

    struct ct_resource_id rid = {.i64 = asset};


    uint32_t idx = ct_hash_lookup(&_G.on_asset_map, rid.type, UINT32_MAX);

    _G.active_on_asset = UINT32_MAX != idx ? _G.on_asset[idx] : NULL;
    _G.active_asset = rid;
    _G.active_path = path;

    ct_property_editor_a0->set_active(on_debugui);
}

static struct ct_asset_property_a0 asset_property_api = {
        .register_asset = register_asset,
};

struct ct_asset_property_a0 *ct_asset_property_a0 = &asset_property_api;

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0->main_allocator()
    };

    api->register_api("ct_asset_property_a0", &asset_property_api);

    ct_ebus_a0->connect(ASSET_BROWSER_EBUS, ASSET_DCLICK_EVENT, set_asset, 0);

}

static void _shutdown() {
    ct_array_free(_G.on_asset, _G.allocator);
    ct_hash_free(&_G.on_asset_map, _G.allocator);

    ct_ebus_a0->disconnect(ASSET_BROWSER_EBUS, ASSET_DCLICK_EVENT, set_asset);

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        asset_property,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_property_editor_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_cdb_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)