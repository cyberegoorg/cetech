#ifndef CETECH_RENDERER_H
#define CETECH_RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

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


typedef struct ct_world_t0 ct_world_t0;
typedef struct ct_entity_t0 ct_entity_t0;
typedef struct ct_rg_module ct_rg_module;
typedef struct ct_rg_builder_t0 ct_rg_builder_t0;

typedef void (*ct_renderender_on_render)();

typedef struct ct_renderer_component_i0 {
    void (*feed_module)(ct_world_t0 world,
                        ct_rg_module *module);

    void (*render)(ct_world_t0 world,
                   ct_rg_builder_t0 *builder);
} ct_renderer_component_i0;

typedef struct ct_viewport_t0 {
    uint32_t idx;
} ct_viewport_t0;


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

    struct ct_viewport_t0 (*create_viewport)(ct_world_t0 world,
                                           struct ct_entity_t0 main_camera);

    void (*destroy_viewport)(struct ct_viewport_t0 viewport);

    struct ct_rg_builder_t0 *(*viewport_builder)(struct ct_viewport_t0 viewport);

    struct ce_window *(*get_main_window)();
};

CE_MODULE(ct_renderer_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_RENDERER_H

