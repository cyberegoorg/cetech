#include <celib/macros.h>
#include <celib/module.h>
#include <celib/memory.h>
#include <celib/hashlib.h>
#include <celib/api_system.h>
#include <celib/cdb.h>
#include <cetech/node_graph_editor/node_graph_editor.h>

#include "../node_graph.h"

#define _G node_test_globals
static struct _G {
    struct ce_alloc *allocator;
} _G;


static const char *display_name() {
    return "Test node";
}


struct ct_editor_node_i0 editor_node_i0 = {
        .display_name  = display_name,
};


static uint64_t cdb_type() {
    return ce_id_a0->id64("node_test");
}

static void *get_interface(uint64_t name_hash) {
    if (name_hash == CT_EDITOR_NODE_I) {
        return &editor_node_i0;
    }
    return NULL;
}

const struct ct_node_pin_def *input_defs(uint32_t *n) {
    static struct ct_node_pin_def def[] = {
            {.name= "IN 1"},
            {.name= "IN 2"},
    };

    *n = CE_ARRAY_LEN(def);
    return def;
}

const struct ct_node_pin_def *output_defs(uint32_t *n) {
    static struct ct_node_pin_def def[] = {
            {.name= "OUT 1"},
    };

    *n = CE_ARRAY_LEN(def);
    return def;
}

struct ct_node_i0 node_i0 = {
        .cdb_type = cdb_type,
        .get_interface = get_interface,
        .input_defs = input_defs,
        .output_defs = output_defs,
};


void CE_MODULE_INITAPI(node_test)(struct ce_api_a0 *api) {
}

void CE_MODULE_LOAD (node_test)(struct ce_api_a0 *api,
                                int reload) {
    CE_UNUSED(reload);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->register_api(CT_NODE_I, &node_i0);

}

void CE_MODULE_UNLOAD (node_test)(struct ce_api_a0 *api,
                                  int reload) {

    CE_UNUSED(api);
}