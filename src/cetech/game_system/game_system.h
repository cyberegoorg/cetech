#ifndef CETECH_GAME_H
#define CETECH_GAME_H

#include <stdint.h>

#include <corelib/module.inl>
#include <cetech/gfx/render_graph.h>

#define GAME_INTERFACE_NAME \
    "ct_game_i0"

#define GAME_INTERFACE \
    CT_ID64_0("ct_game_i0", 0x22500b95a05f8b37ULL)


struct ct_render_texture_handle;
struct ct_render_graph_builder;

struct ct_game_i0 {
    uint64_t (*name)();

    void (*init)();

    void (*shutdown)();

    void (*update)(float dt);

    void (*render)();

    struct ct_render_graph_builder *(*render_graph_builder)();
};

struct ct_game_system_a0 {
    struct ct_render_graph_builder *(*render_graph_builder)(uint64_t name);

    void (*pause)(uint64_t name);

    bool (*is_paused)(uint64_t name);

    void (*play)(uint64_t name);

    void (*step)(uint64_t name, float dt);
};

CT_MODULE(ct_game_system_a0);

#endif //CETECH_GAME_H
