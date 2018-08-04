#include <stdio.h>

#include <celib/macros.h>
#include <celib/allocator.h>
#include <celib/fs.h>
#include <celib/os.h>
#include <celib/ydb.h>
#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"


#include <cetech/editor/selected_object.h>

#define _G selected_object_global

static struct _G {
    uint64_t selected_object;
} _G;

static uint64_t selected_object() {
    return _G.selected_object;
}

static void set_selected_object(uint64_t object) {
    _G.selected_object = object;
}

static struct ct_selected_object_a0 selcted_object_api = {
        .selected_object = selected_object,
        .set_selected_object = set_selected_object,
};

struct ct_selected_object_a0 *ct_selected_object_a0 = &selcted_object_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {};

    api->register_api("ct_selected_object_a0", &selcted_object_api);

}

static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        selected_object,
        {

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