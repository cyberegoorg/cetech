#include <stdio.h>

#include <corelib/macros.h>
#include <corelib/allocator.h>
#include <corelib/fs.h>
#include <corelib/os.h>
#include <corelib/ydb.h>
#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/module.h"


#include <cetech/selected_object/selected_object.h>

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

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {};

    api->register_api("ct_selected_object_a0", &selcted_object_api);

}

static void _shutdown() {
    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        selected_object,
        {

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