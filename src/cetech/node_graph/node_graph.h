#ifndef CETECH_NODE_GRAPH_H
#define CETECH_NODE_GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CT_NODE_GRAPH_A0_STR "ct_node_graph_a0"

#define CT_NODE_GRAPH_API \
    CE_ID64_0("ct_node_graph_a0", 0x83f8b5f2c9f24145ULL)

#define CT_NODE_I0_STR "ct_node_i0"

#define CT_NODE_I0 \
    CE_ID64_0("ct_node_i0", 0x31747424097dbbefULL)

#define CT_NODE_GRAPH_ASSET \
    CE_ID64_0("node_graph", 0xfdc584c720df8500ULL)

#define CT_NODE_GRAPH_NODE \
    CE_ID64_0("node_graph_node", 0xaad1c761f478e53aULL)

#define CT_NODE_GRAPH_NODE_POS_X \
    CE_ID64_0("position_x", 0xc8e333b60b9cb460ULL)

#define CT_NODE_GRAPH_NODE_POS_Y \
    CE_ID64_0("position_y", 0xe1984f910d0f8b91ULL)

#define CT_NODE_GRAPH_NODE_SIZE_X \
    CE_ID64_0("size_x", 0xacc8adbd22d2416bULL)

#define CT_NODE_GRAPH_NODE_SIZE_Y \
    CE_ID64_0("size_y", 0x1211758e2a991897ULL)


#define CT_NODE_TYPE \
    CE_ID64_0("type", 0xa21bd0e01ac8f01fULL)

#define CT_NODE_GRAPH_NODES\
    CE_ID64_0("nodes", 0x6ea600aa4b4a3195ULL)

#define CT_NODE_GRAPH_CONNECTIONS\
    CE_ID64_0("connections", 0x4970db49097daf8dULL)

#define CT_NODE_GRAPH_NODE_INPUTS\
    CE_ID64_0("inputs", 0x827c802bac814485ULL)

#define CT_NODE_GRAPH_NODE_OUTPUTS\
    CE_ID64_0("outputs", 0x8ac15af4f96392a6ULL)

#define CT_NODE_GRAPH_CONN_FROM\
    CE_ID64_0("from", 0x5b149b8e28f66e68ULL)

#define CT_NODE_GRAPH_CONN_FROM_PIN\
    CE_ID64_0("from_pin", 0xfaa1f325909fb075ULL)

#define CT_NODE_GRAPH_CONN_TO\
    CE_ID64_0("to", 0x9558f59f36417bc6ULL)

#define CT_NODE_GRAPH_CONN_TO_PIN\
    CE_ID64_0("to_pin", 0x56c235c40d1418d5ULL)

typedef enum ct_node_pin_type_e0 {
    CT_NODE_PIN_NONE = 0,
    CT_NODE_PIN_FLOAT,
    CT_NODE_PIN_STRING,
    CT_NODE_PIN_BOOL,
} ct_node_pin_type_e0;

typedef struct ct_node_pin_def {
    enum ct_node_pin_type_e0 type;
    const char *name;
    uint64_t prop;
} ct_node_pin_def;

typedef struct ct_node_i0 {
    uint64_t (*type)();

    void *(*get_interface)(uint64_t name_hash);

    const ct_node_pin_def *(*input_defs)(uint32_t *n);

    const ct_node_pin_def *(*output_defs)(uint32_t *n);

} ct_node_i0;

struct ct_node_graph_a0 {
    ct_node_i0 *(*get_interface)(uint64_t type);
};

CE_MODULE(ct_node_graph_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_NODE_GRAPH_H
