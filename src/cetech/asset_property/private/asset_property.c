#include <cetech/debugui/debugui.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/asset_property/asset_property.h>
#include <cetech/resource/resource.h>
#include <cetech/asset_browser/asset_browser.h>

#include <corelib/hash.inl>
#include <corelib/ebus.h>
#include <corelib/ydb.h>
#include <cetech/selected_object/selected_object.h>

#include "corelib/hashlib.h"
#include "corelib/config.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/module.h"

#define _G asset_property_global
static struct _G {
    struct ct_hash_t on_asset_map;

    struct ct_resource_id active_asset;

    const char *active_path;
    struct ct_alloc *allocator;
} _G;

static void draw_ui(uint64_t obj) {
    if (!obj) {
        return;
    }

    uint64_t obj_type = ct_cdb_a0->type(obj);

    uint64_t res_obj = 0;
    if (obj_type == ASSET_BROWSER_ASSET_TYPE) {

        uint64_t asset_type = ct_cdb_a0->read_uint64(obj,
                                                     ASSET_BROWSER_ASSET_TYPE2,
                                                     0);
        uint64_t asset_name = ct_cdb_a0->read_uint64(obj,
                                                     ASSET_BROWSER_ASSET_NAME,
                                                     0);
        struct ct_resource_id rid = (struct ct_resource_id){
            .name = asset_name,
            .type = asset_type
        };

        res_obj = ct_resource_a0->get(rid);
    } else {
        res_obj = obj;
    }


//    char filename[512] = {};
//    ct_resource_a0->compiler_get_filename(filename,
//                                          CT_ARRAY_LEN(filename),
//                                          rid);

//    if (ct_debugui_a0->Button("Save", (float[2]) {0.0f})) {
//        ct_ydb_a0->save(filename);
//    }
//    ct_debugui_a0->SameLine(0.0f, -1.0f);
//
//    ct_debugui_a0->InputText("asset",
//                             filename, strlen(filename),
//                             DebugInputTextFlags_ReadOnly, 0, NULL);

    if(!res_obj) {
        return;
    }

    uint64_t res_obj_type = ct_cdb_a0->type(res_obj);

    struct ct_resource_i0 *resource_i = ct_resource_a0->get_interface(res_obj_type);
    if (!resource_i) {
        return;
    }

    if (!resource_i->get_interface) {
        return;
    }

    struct ct_asset_property_i0 *i = resource_i->get_interface(ASSET_PROPERTY);
    if (!i) {
        return;
    }

    if (!i->draw) {
        return;
    }

    const char* display_name = "";

    if(i->display_name) {
        display_name = i->display_name();
    }

    if (ct_debugui_a0->TreeNodeEx(display_name,
                                  DebugUITreeNodeFlags_DefaultOpen)) {
        i->draw(res_obj);

        ct_debugui_a0->TreePop();
    }
}


static struct ct_asset_property_a0 asset_property_api = {
};

struct ct_asset_property_a0 *ct_asset_property_a0 = &asset_property_api;


static struct ct_property_editor_i0 ct_property_editor_i0 = {
        .draw_ui = draw_ui,
};

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0->system
    };

    api->register_api("ct_asset_property_a0", &asset_property_api);
    api->register_api(PROPERTY_EDITOR_INTERFACE_NAME, &ct_property_editor_i0);
}

static void _shutdown() {
    ct_hash_free(&_G.on_asset_map, _G.allocator);

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        asset_property,
        {
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_hashlib_a0);
            CT_INIT_API(api, ct_debugui_a0);
            CT_INIT_API(api, ct_resource_a0);
            CT_INIT_API(api, ct_ydb_a0);
            CT_INIT_API(api, ct_ebus_a0);
            CT_INIT_API(api, ct_cdb_a0);
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