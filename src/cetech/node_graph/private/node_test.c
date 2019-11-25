#include <celib/macros.h>
#include <celib/module.h>
#include <celib/memory/memory.h>
#include <celib/id.h>
#include <celib/api.h>
#include <celib/cdb.h>
#include <cetech/node_graph_editor/node_graph_editor.h>

#include "../node_graph.h"

#define _G node_test_globals
static struct _G {
    ce_alloc_t0 *allocator;
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
    if (name_hash == CT_EDITOR_NODE_I0) {
        return &editor_node_i0;
    }
    return NULL;
}

#define _PROP_IN1\
    CE_ID64_0("in_1", 0xd357f354a0ff6e1dULL)

#define _PROP_IN2\
    CE_ID64_0("in_2", 0x9ff9657cbc3c26abULL)

#define _PROP_IN3\
    CE_ID64_0("in_3", 0xc32c0532c5479e19ULL)

#define _PROP_IN4\
    CE_ID64_0("in_4", 0x8782c563666f2d2cULL)

const struct ct_node_pin_def *input_defs(uint32_t *n) {
    static struct ct_node_pin_def def[] = {
            {.name= "IN 1", .type = CT_NODE_PIN_FLOAT, .prop = _PROP_IN1},
            {.name= "IN 2", .type = CT_NODE_PIN_STRING, .prop = _PROP_IN2},
            {.name= "IN 3", .type = CT_NODE_PIN_BOOL, .prop = _PROP_IN3},
    };

    *n = CE_ARRAY_LEN(def);
    return def;
}

const struct ct_node_pin_def *output_defs(uint32_t *n) {
    static struct ct_node_pin_def def[] = {
            {.name= "OUT 1", .type = CT_NODE_PIN_FLOAT},
            {.name= "OUT 2", .type = CT_NODE_PIN_FLOAT},
            {.name= "OUT 3", .type = CT_NODE_PIN_STRING},
    };

    *n = CE_ARRAY_LEN(def);
    return def;
}

struct ct_node_i0 node_i0 = {
        .type = cdb_type,
        .get_interface = get_interface,
        .input_defs = input_defs,
        .output_defs = output_defs,
};


void CE_MODULE_LOAD (node_test)(struct ce_api_a0 *api,
                                int reload) {
    CE_UNUSED(reload);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->add_impl(CT_NODE_I0_STR, &node_i0, sizeof(node_i0));

}

void CE_MODULE_UNLOAD (node_test)(struct ce_api_a0 *api,
                                  int reload) {

    CE_UNUSED(api);
}