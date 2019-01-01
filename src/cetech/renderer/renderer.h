#ifndef CETECH_RENDERER_H
#define CETECH_RENDERER_H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#define CT_RENDERER_API \
    CE_ID64_0("ct_renderer_a0", 0xfe29b15a8b749c94ULL)

#define CONFIG_RENDERER_TYPE \
     CE_ID64_0("renderer.type", 0x3ee74a7da01daff0ULL)

#define CONFIG_WID \
     CE_ID64_0("screen.wid", 0xd950bcde34e3e4d7ULL)

#define CONFIG_SCREEN_X \
     CE_ID64_0("screen.x", 0xa59339bef4754edaULL)

#define CONFIG_SCREEN_Y \
     CE_ID64_0("screen.y", 0xc3492a4e07168b7aULL)

#define CONFIG_SCREEN_VSYNC \
     CE_ID64_0("screen.vsync", 0xfe469e6d1c14c940ULL)

#define CONFIG_SCREEN_FULLSCREEN \
     CE_ID64_0("screen.fullscreen", 0x613e9a6a17148a72ULL)

#define CT_RENDER_TASK \
     CE_ID64_0("render_task", 0x193bb679e415e81bULL)

#define CT_RENDERER_COMPONENT_I \
     CE_ID64_0("ct_renderer_component_i0", 0xe832f6a0542ec6a0ULL)

typedef void (*ct_renderender_on_render)();

struct ct_world;
struct ct_entity;
struct ct_rg_module;
struct ct_rg_builder;

struct ct_renderer_component_i0 {
    void (*feed_module)(struct ct_world world,
                        struct ct_rg_module *module);

    void (*render)(struct ct_world world,
                   struct ct_rg_builder *builder);
};

struct ct_viewport0 {
    uint32_t idx;
};


//! Render API V0
struct ct_renderer_a0 {
    //! Create renderer.
    void (*create)();

    //! Set debug mode on/off
    //! \param debug True/False
    void (*set_debug)(int debug);

    //! Get renderer window size
    //! \return Renderer window size
    void (*get_size)(uint32_t *width,
                     uint32_t *height);

    uint64_t (*new_viewid)();

    struct ct_viewport0 (*create_viewport)(struct ct_world world,
                                           struct ct_entity main_camera);

    struct ct_rg_builder *
    (*viewport_builder)(struct ct_viewport0 viewport);
};

CE_MODULE(ct_renderer_a0);

#endif //CETECH_RENDERER_H
//! \}
