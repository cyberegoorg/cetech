#ifndef CETECH_NODE_GRAPH_EDITOR_H
#define CETECH_NODE_GRAPH_EDITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CT_NODE_GRAPH_EDITOR_A0_STR "ct_node_graph_editor_a0"

#define CT_NODE_GRAPH_EDITOR_API \
    CE_ID64_0("ct_node_graph_editor_a0", 0xc13af81eb433e221ULL)


#define CT_EDITOR_NODE_I0 \
    CE_ID64_0("ct_editor_node_i0", 0x3cb5090f34945e9bULL)


typedef struct ct_editor_node_i0 {
    const char *(*display_name)();
} ct_editor_node_i0;


struct ct_node_graph_editor_a0 {
    void (*add_node_modal)(const char *modal_id,
                           uint64_t obj);

    void (*draw_ng_editor)(uint64_t graph,
                           uint64_t context);
};

CE_MODULE(ct_node_graph_editor_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_NODE_GRAPH_EDITOR_H
