#include "celib/map.inl"

#include <cetech/debugui/debugui.h>
#include <cetech/playground/property_inspector.h>
#include <cetech/playground/level_inspector.h>
#include <cetech/os/vio.h>
#include <cetech/filesystem/filesystem.h>
#include <cetech/yaml/yaml.h>
#include <cetech/playground/asset_browser.h>

#include "cetech/hashlib/hashlib.h"
#include "cetech/config/config.h"
#include "cetech/memory/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_asset_browser_a0);

using namespace celib;

#define _G property_inspector_global
static struct _G {
    bool visible;

    yaml_document_t h;
    yaml_node_t root;
    uint64_t level;
} _G;


void set_level(uint64_t name, uint64_t root, const char* path) {
    if( _G.level == name) {
        return;
    }

    _G.level = name;

    ct_vio *f = ct_filesystem_a0.open(root, path, FS_OPEN_READ);
    char source_data[f->size(f->inst) + 1];
    memset(source_data, 0, f->size(f->inst) + 1);
    f->read(f->inst, source_data, sizeof(char), f->size(f->inst));
    ct_filesystem_a0.close(f);

    _G.root = yaml_load_str(source_data, &_G.h);
}

static ct_level_inspector_a0 level_inspector_api = {
        .set_level = set_level
};


static void on_gui() {
    if (ct_debugui_a0.BeginDock("Level inspector", &_G.visible,
                                DebugUIWindowFlags_(0))) {

        ct_debugui_a0.LabelText("Level", "%lu", _G.level);

        if(yaml_is_valid(_G.root)) {
            yaml_node_foreach_dict(
                    _G.root,
                    [](yaml_node_t key,
                       yaml_node_t value,
                       void *_data) {
                        char str_buffer[128] = {};

                        yaml_as_string(key, str_buffer,
                                       CETECH_ARRAY_LEN(str_buffer) - 1);

                        if (ct_debugui_a0.TreeNodeEx(str_buffer, DebugUITreeNodeFlags_DefaultOpen)) {
                            yaml_node_foreach_dict(
                                    value,
                                    [](yaml_node_t key,
                                       yaml_node_t value,
                                       void *_data) {

                                        char str_buffer[128] = {};
                                        yaml_as_string(key, str_buffer,
                                                       CETECH_ARRAY_LEN(str_buffer) - 1);

                                        if (ct_debugui_a0.Selectable(str_buffer, false, 0, (float[]){0.0f, 0.0f})) {

                                        }

                                    }, NULL);


                            ct_debugui_a0.TreePop();
                        }

                        yaml_node_free(value);
                    }, NULL);
        }
    }
    ct_debugui_a0.EndDock();
}

static void _init(ct_api_a0 *api) {
    _G = {
            .visible = true
    };

    api->register_api("ct_level_inspector_a0", &level_inspector_api);
    ct_debugui_a0.register_on_gui(on_gui);
}

static void _shutdown() {
    _G = {};
}

CETECH_MODULE_DEF(
        level_inspector,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
        },
        {
            _init(api);
        },
        {
            CEL_UNUSED(api);
            _shutdown();
        }
)