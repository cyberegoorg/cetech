#include "celib/map.inl"

#include <cetech/debugui/debugui.h>
#include <cetech/playground/property_editor.h>
#include <cetech/playground/asset_property.h>
#include <cetech/resource/resource.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/debugui/private/ocornut-imgui/imgui.h>
#include <celib/hash.h>

#include "cetech/hashlib/hashlib.h"
#include "cetech/config/config.h"
#include "cetech/os/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_property_editor_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_ydb_a0);

using namespace celib;

#define _G asset_property_global
static struct _G {
    cel_hash_t on_asset_map;
    ct_ap_on_asset* on_asset;

    ct_ap_on_asset active_on_asset;
    uint64_t active_type;
    uint64_t active_name;
    const char *active_path;
    struct cel_alloc* allocator;
} _G;

static void on_debugui() {
    char filename[512] = {};
    ct_resource_a0.compiler_get_filename(filename, CETECH_ARRAY_LEN(filename),
                                         _G.active_type, _G.active_name);

    if (ct_debugui_a0.Button("Save", (float[2]) {0.0f})) {
        ct_ydb_a0.save(filename);
    }
    ct_debugui_a0.SameLine(0.0f, -1.0f);

    ct_debugui_a0.InputText("asset",
                            filename, strlen(filename),
                            DebugInputTextFlags_ReadOnly, 0, NULL);

    if (_G.active_on_asset) {
        _G.active_on_asset(_G.active_type, _G.active_name, _G.active_path);
    }
}

static void register_asset(uint64_t type,
                           ct_ap_on_asset on_asset) {
    cel_array_push(_G.on_asset, on_asset, _G.allocator);
    cel_hash_add(&_G.on_asset_map, type, cel_array_size(_G.on_asset) - 1, _G.allocator);
}

static void set_asset(uint64_t type,
                      uint64_t name,
                      uint64_t root,
                      const char *path) {
    CEL_UNUSED(root);

    uint32_t idx = cel_hash_lookup(&_G.on_asset_map, type, UINT32_MAX);

    _G.active_on_asset = UINT32_MAX != idx ? _G.on_asset[idx]: NULL;
    _G.active_type = type;
    _G.active_name = name;
    _G.active_path = path;

    ct_property_editor_a0.set_active(on_debugui);
}

static ct_asset_property_a0 asset_property_api = {
        .register_asset = register_asset,
};


static void _init(ct_api_a0 *api) {
    _G = {
            .allocator = ct_memory_a0.main_allocator()
    };

    api->register_api("ct_asset_property_a0", &asset_property_api);

    ct_asset_browser_a0.register_on_asset_double_click(set_asset);
}

static void _shutdown() {
    cel_array_free(_G.on_asset, _G.allocator);
    cel_hash_free(&_G.on_asset_map, _G.allocator);

    ct_asset_browser_a0.unregister_on_asset_double_click(set_asset);

    _G = {};
}

CETECH_MODULE_DEF(
        asset_property,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_property_editor_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_ydb_a0);
        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            _shutdown();
        }
)