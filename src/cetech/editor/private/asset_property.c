#include <cetech/gfx/debugui.h>
#include <cetech/editor/property_editor.h>
#include <cetech/editor/asset_property.h>
#include <cetech/resource/resource.h>
#include <cetech/editor/asset_browser.h>

#include <celib/hash.inl>
#include <celib/ebus.h>
#include <celib/ydb.h>
#include <cetech/resource/sourcedb.h>
#include <celib/cdb.h>

#include "celib/hashlib.h"
#include "celib/config.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"

#define _G asset_property_global
static struct _G {
    struct ce_hash_t on_asset_map;

    struct ct_resource_id active_asset;

    const char *active_path;
    struct ce_alloc *allocator;
} _G;

static void draw_ui(struct ct_resource_id rid, uint64_t obj) {
    if (!obj) {
        return;
    }

    uint64_t res_obj_type = ce_cdb_a0->type(obj);

    struct ct_resource_i0 *resource_i = ct_resource_a0->get_interface(
            res_obj_type);
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

    i->draw(rid, obj);

}


static struct ct_asset_property_a0 asset_property_api = {
};

struct ct_asset_property_a0 *ct_asset_property_a0 = &asset_property_api;


static struct ct_property_editor_i0 ct_property_editor_i0 = {
        .draw_ui = draw_ui,
};

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system
    };

    api->register_api("ct_asset_property_a0", &asset_property_api);
    api->register_api(PROPERTY_EDITOR_INTERFACE_NAME, &ct_property_editor_i0);
}

static void _shutdown() {
    ce_hash_free(&_G.on_asset_map, _G.allocator);

    _G = (struct _G) {};
}

CE_MODULE_DEF(
        asset_property,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ce_cdb_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)
