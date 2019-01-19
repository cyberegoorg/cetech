#ifndef CETECH_GAME_H
#define CETECH_GAME_H

#include <stdint.h>

#include <celib/module.inl>
#include <cetech/render_graph/render_graph.h>

#define CT_GAME_SYSTEM_API \
    CE_ID64_0("ct_game_system_a0", 0x1a5b2ed4808612b9ULL)

#define GAME_INTERFACE \
    CE_ID64_0("ct_game_i0", 0x22500b95a05f8b37ULL)

#define CT_GAME_TASK \
    CE_ID64_0("game_task", 0x56c73acaa12b1278ULL)

struct ct_rg_builder;

struct ct_viewport0;

struct ct_game_i0 {
    uint64_t (*name)();

    void (*init)();

    void (*shutdown)();

    void (*update)(float dt);

    struct ct_viewport0 (*render_graph_builder)();
};

struct ct_game_system_a0 {
    struct ct_viewport0 (*viewport)(uint64_t name);

    void (*pause)(uint64_t name);

    bool (*is_paused)(uint64_t name);

    void (*play)(uint64_t name);

    void (*step)(uint64_t name, float dt);
};

CE_MODULE(ct_game_system_a0);

#endif //CETECH_GAME_H
