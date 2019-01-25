#ifndef CETECH_NODE_GRAPH_H
#define CETECH_NODE_GRAPH_H


#include <stdint.h>
#include <celib/module.inl>

#define CT_NODE_GRAPH_API \
    CE_ID64_0("ct_node_graph_a0", 0x83f8b5f2c9f24145ULL)

#define CT_NODE_I \
    CE_ID64_0("ct_node_i0", 0x31747424097dbbefULL)

#define CT_NODE_GRAPH_RESOURCE \
    CE_ID64_0("node_graph", 0xfdc584c720df8500ULL)

#define CT_NODE_GRAPH_NODE \
    CE_ID64_0("node_graph_node", 0xaad1c761f478e53aULL)

#define CT_NODE_GRAPH_NODE_POS_X \
    CE_ID64_0("position_x", 0xc8e333b60b9cb460ULL)

#define CT_NODE_GRAPH_NODE_POS_Y \
    CE_ID64_0("position_y", 0xe1984f910d0f8b91ULL)

#define CT_NODE_TYPE \
    CE_ID64_0("type", 0xa21bd0e01ac8f01fULL)

#define CT_NODE_GRAPH_NODES\
    CE_ID64_0("nodes", 0x6ea600aa4b4a3195ULL)

#define CT_NODE_GRAPH_CONNECTIONS\
    CE_ID64_0("connections", 0x4970db49097daf8dULL)


struct ct_node_pin_def {
    const char *name;
};

struct ct_node_i0 {
    uint64_t (*cdb_type)();

    void *(*get_interface)(uint64_t name_hash);

    const struct ct_node_pin_def *(*input_defs)(uint32_t *n);
    const struct ct_node_pin_def *(*output_defs)(uint32_t *n);

};

struct ct_node_graph_a0 {
    struct ct_node_i0 *(*get_interface)(uint64_t type);
};

CE_MODULE(ct_node_graph_a0);

#endif //CETECH_NODE_GRAPH_H
