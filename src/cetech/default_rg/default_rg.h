#ifndef CETECH_DEFAULT_RENDER_GRAPH_H
#define CETECH_DEFAULT_RENDER_GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CT_DEFAULT_RG_API \
    CE_ID64_0("ct_default_rg_a0", 0xe909f984404895b4ULL)

typedef struct ct_rg_module ct_rg_module;
typedef struct ct_world_t0 ct_world_t0;

#define _GBUFFER \
     CE_ID64_0("gbuffer", 0xc0d9fff4f568ebfdULL)

#define _DEFAULT \
     CE_ID64_0("default", 0xf27605035974b5ecULL)

#define _COLOR \
    CE_ID64_0("color", 0x6776ddaf0290228ULL)

#define _DEPTH \
     CE_ID64_0("depth", 0x911ffdcbfa56fec1ULL)


struct ct_default_rg_a0 {
    void (*feed_module)(ct_rg_module *m,
                        ct_world_t0 world,
                        ct_entity_t0 camera);
};

#ifdef __cplusplus
};
#endif

CE_MODULE(ct_default_rg_a0);

#endif //CETECH_DEFAULT_RENDER_GRAPH_H
