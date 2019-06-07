#include <celib/macros.h>
#include <celib/module.h>
#include <celib/memory.h>
#include <celib/hashlib.h>
#include <celib/api_system.h>

#include "../node_graph.h"

#define _G node_graph_globals
static struct _G {
    struct ce_alloc *allocator;
} _G;


static struct ct_node_graph_a0 ng_api = {
};

struct ct_node_graph_a0 *ct_node_graph_a0 = &ng_api;


void CE_MODULE_INITAPI(node_graph)(struct ce_api_a0 *api) {
}

void CE_MODULE_LOAD (node_graph)(struct ce_api_a0 *api,
                                 int reload) {
    CE_UNUSED(reload);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->register_api(CT_NODE_GRAPH_API, ct_node_graph_a0);
}

void CE_MODULE_UNLOAD (node_graph)(struct ce_api_a0 *api,
                                   int reload) {

    CE_UNUSED(api);
}