#ifndef CETECH_DEFAULT_RENDER_GRAPH_H
#define CETECH_DEFAULT_RENDER_GRAPH_H


#include <stdint.h>

#define CT_DEFAULT_RG_API \
    CE_ID64_0("ct_default_rg_a0", 0xe909f984404895b4ULL)

struct ct_rg_module;
struct ct_world;


#define _GBUFFER \
     CE_ID64_0("gbuffer", 0xc0d9fff4f568ebfdULL)

#define _DEFAULT \
     CE_ID64_0("default", 0xf27605035974b5ecULL)

#define _COLOR \
    CE_ID64_0("color", 0x6776ddaf0290228ULL)

#define _DEPTH \
     CE_ID64_0("depth", 0x911ffdcbfa56fec1ULL)


struct ct_default_rg_a0 {
    void (*feed_module)(struct ct_rg_module *m,
                        struct ct_world world,
                        struct ct_entity camera);
};

CE_MODULE(ct_default_rg_a0);

#endif //CETECH_DEFAULT_RENDER_GRAPH_H
