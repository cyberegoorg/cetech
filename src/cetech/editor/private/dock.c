#include <stdbool.h>

#include <celib/array.inl>
#include <celib/hashlib.h>
#include <celib/memory.h>
#include <celib/api_system.h>
#include <celib/module.h>
#include <celib/cdb.h>

#include <cetech/debugui/debugui.h>
#include <cetech/editor/property.h>
#include <stdio.h>

#include "../dock.h"

#define _G explorer_globals
static struct _G {
    uint64_t docks_n;
    uint64_t *docks;
    struct ce_alloc *allocator;
} _G;

#define _PROP_DOCK_ID\
    CE_ID64_0("dock_id", 0x4a6df3bdedc53da2ULL)

uint64_t create_dock(uint64_t type,
                     enum DebugUIWindowFlags_ flags,
                     bool visible) {
    uint64_t obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), type);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
    ce_cdb_a0->set_bool(w, PROP_DOCK_VISIBLE, visible);
    ce_cdb_a0->set_uint64(w, PROP_DOCK_flags, (uint64_t) flags);
    ce_cdb_a0->set_uint64(w, _PROP_DOCK_ID, _G.docks_n++);
    ce_cdb_a0->write_commit(w);

    ce_array_push(_G.docks, obj, _G.allocator);

    return obj;
}

struct ct_dock_i0 *_find_dock_i(uint64_t type) {
    struct ce_api_entry it = ce_api_a0->first(DOCK_INTERFACE);

    while (it.api) {
        struct ct_dock_i0 *i = (it.api);

        if (i && i->cdb_type
            && (i->cdb_type() == type)) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
}

void draw_all() {
    uint64_t n = ce_array_size(_G.docks);

    for (int i = 0; i < n; ++i) {
        uint64_t dock = _G.docks[i];
        uint64_t type = ce_cdb_a0->obj_type(dock);
        struct ct_dock_i0 *di = _find_dock_i(type);

        if (di) {
            uint64_t id = ce_cdb_a0->read_uint64(dock, _PROP_DOCK_ID, 0);
            char title[128] = {};
            snprintf(title, CE_ARRAY_LEN(title), "%s##%s_dock%llu",
                     di->display_title(dock), di->name(dock), id);

            uint64_t flags = ce_cdb_a0->read_uint64(dock, PROP_DOCK_flags, 0);
            bool visible = ce_cdb_a0->read_bool(dock, PROP_DOCK_VISIBLE, false);

            if (ct_debugui_a0->BeginDock(title, &visible, flags)) {
                if (di->draw_menu) {
                    di->draw_menu(dock);
                }

                if (di->draw_ui) {
                    di->draw_ui(dock);
                }
            }
            ct_debugui_a0->EndDock();
        }
    }
}


struct ct_dock_a0 dock_a0 = {
        .draw_all = draw_all,
        .create_dock = create_dock,
};

struct ct_dock_a0 *ct_dock_a0 = &dock_a0;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ce_api_a0->register_api("ct_dock_a0", ct_dock_a0);

}

static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        dock,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_module_a0);
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