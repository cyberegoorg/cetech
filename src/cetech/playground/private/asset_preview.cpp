#include <cetech/entity/entity.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/texture.h>
#include <cetech/debugui/debugui.h>
#include <cetech/application/application.h>
#include <cetech/level/level.h>
#include <cetech/camera/camera.h>
#include <cetech/transform/transform.h>

#include <cetech/input/input.h>
#include <cetech/renderer/viewport.h>
#include <cetech/playground/asset_preview.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/playground/playground.h>
#include <celib/hash.h>
#include <celib/fmath.h>
#include "celib/map.inl"

#include "cetech/hashlib/hashlib.h"
#include "cetech/config/config.h"
#include "cetech/os/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_keyboard_a0);
CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_viewport_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_playground_a0);

using namespace celib;

#define _G AssetPreviewGlobals

static struct _G {
    cel_alloc* allocator;
    struct cel_hash_t preview_fce_map;
    ct_asset_preview_fce* preview_fce;

    uint64_t active_type;
    uint64_t active_name;
    const char *active_path;

    ct_viewport viewport;
    ct_world world;
    ct_entity camera_ent;
    bool visible;
    bool active;
} _G;


static void fps_camera_update(ct_world world,
                              ct_entity camera_ent,
                              float dt,
                              float dx,
                              float dy,
                              float updown,
                              float leftright,
                              float speed,
                              bool fly_mode) {

    CEL_UNUSED(dx);
    CEL_UNUSED(dy);

    float pos[3];
    float rot[4];
    float wm[16];

    auto transform = ct_transform_a0.get(world, camera_ent);

    ct_transform_a0.get_position(transform, pos);
    ct_transform_a0.get_rotation(transform, rot);
    ct_transform_a0.get_world_matrix(transform, wm);

    float x_dir[4];
    float z_dir[4];
    cel_vec4_move(x_dir, &wm[0 * 4]);
    cel_vec4_move(z_dir, &wm[2 * 4]);


    if (!fly_mode) {
        z_dir[1] = 0.0f;
    }

    // POS
    float x_dir_new[3];
    float z_dir_new[3];

    cel_vec3_mul_s(x_dir_new, x_dir, dt * leftright * speed);
    cel_vec3_mul_s(z_dir_new, z_dir, dt * updown * speed);

    cel_vec3_add(pos, pos, x_dir_new);
    cel_vec3_add(pos, pos, z_dir_new);

    ct_transform_a0.set_position(transform, pos);
}

static void on_debugui() {
    if (ct_debugui_a0.BeginDock("Asset preview", &_G.visible,
                                DebugUIWindowFlags_(
                                        DebugUIWindowFlags_NoScrollbar))) {

        _G.active = ct_debugui_a0.IsMouseHoveringWindow();

        auto th = ct_viewport_a0.get_local_resource(
                _G.viewport, CT_ID64_0("bb_color"));

        float size[2];
        ct_debugui_a0.GetWindowSize(size);
        ct_viewport_a0.resize(_G.viewport, size[0], size[1]);
        ct_debugui_a0.Image2(th,
                             size,
                             (float[2]) {0.0f, 0.0f},
                             (float[2]) {1.0f, 1.0f},
                             (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                             (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
    }
    ct_debugui_a0.EndDock();

}

static void render() {
    if (_G.visible) {
        ct_camera camera = ct_camera_a0.get(_G.world, _G.camera_ent);
        ct_viewport_a0.render_world(_G.world, camera, _G.viewport);
    }
}

static void set_asset(uint64_t type,
                      uint64_t name,
                      uint64_t root,
                      const char *path) {
    CEL_UNUSED(root);

    if (_G.active_name == name) {
        return;
    }

    uint64_t idx = cel_hash_lookup(&_G.preview_fce_map,_G.active_type, UINT64_MAX);
    if(idx != UINT64_MAX) {
        ct_asset_preview_fce fce = _G.preview_fce[idx];

        if(fce.unload) {
            fce.unload(_G.active_path, _G.active_type, _G.active_name, _G.world);
        }
    }

    _G.active_type = type;
    _G.active_name = name;
    _G.active_path = path;

    idx = cel_hash_lookup(&_G.preview_fce_map, type, UINT64_MAX);
    if(idx != UINT64_MAX) {
        ct_asset_preview_fce fce = _G.preview_fce[idx];

        if (fce.load) {
            fce.load(path, type, name, _G.world);
        }
    }
}

static void init() {
    _G.visible = true;
    _G.viewport = ct_viewport_a0.create(CT_ID64_0("default"), 0, 0);
    _G.world = ct_world_a0.create();
    _G.camera_ent = ct_entity_a0.spawn(_G.world, CT_ID64_0("content/camera"));

    ct_transform t = ct_transform_a0.get(_G.world, _G.camera_ent);
    ct_transform_a0.set_position(t, (float[3]) {0.0f, 0.0f, -10.0f});
}

static void shutdown() {

}

static void update(float dt) {
    if (!_G.active) {
        return;
    }

    float updown = 0.0f;
    float leftright = 0.0f;

    auto up_key = ct_keyboard_a0.button_index("w");
    auto down_key = ct_keyboard_a0.button_index("s");
    auto left_key = ct_keyboard_a0.button_index("a");
    auto right_key = ct_keyboard_a0.button_index("d");

    if (ct_keyboard_a0.button_state(0, up_key) > 0) {
        updown = 1.0f;
    }

    if (ct_keyboard_a0.button_state(0, down_key) > 0) {
        updown = -1.0f;
    }

    if (ct_keyboard_a0.button_state(0, right_key) > 0) {
        leftright = 1.0f;
    }

    if (ct_keyboard_a0.button_state(0, left_key) > 0) {
        leftright = -1.0f;
    }

    fps_camera_update(_G.world, _G.camera_ent, dt,
                      0, 0, updown, leftright, 10.0f, false);
}

#define cel_instance_map(a, h, k, item, al) \
    cel_array_push(a, item, al); \
    cel_hash_add(h, k, cel_array_size(a) - 1, al)

void register_type_preview(uint64_t type,
                           ct_asset_preview_fce fce) {
    cel_instance_map(_G.preview_fce, &_G.preview_fce_map, type, fce, _G.allocator);
}

void unregister_type_preview(uint64_t type) {
    uint64_t idx = cel_hash_lookup(&_G.preview_fce_map, type, UINT64_MAX);
    if(UINT64_MAX == idx){
        return;
    }
    cel_hash_remove(&_G.preview_fce_map, type);
}

static void on_menu_window() {
    ct_debugui_a0.MenuItem2("Asset preview", NULL, &_G.visible, true);
}

static ct_asset_preview_a0 asset_preview_api = {
        .register_type_preview = register_type_preview,
        .unregister_type_preview = unregister_type_preview
};

static void _init(ct_api_a0 *api) {
    _G = (struct _G){
            .allocator = ct_memory_a0.main_allocator()
    };

    ct_playground_a0.register_module(
            CT_ID64_0("asset_preview"),
            (ct_playground_module_fce) {
                    .on_init = init,
                    .on_shutdown = shutdown,
                    .on_render = render,
                    .on_update = update,
                    .on_ui = on_debugui,
                    .on_menu_window = on_menu_window,
            });

    ct_asset_browser_a0.register_on_asset_click(set_asset);

    api->register_api("ct_asset_preview_a0", &asset_preview_api);
}

static void _shutdown() {
    ct_playground_a0.unregister_module(CT_ID64_0("asset_preview"));
    ct_asset_browser_a0.unregister_on_asset_click(set_asset);

    cel_hash_free(&_G.preview_fce_map, _G.allocator);
    cel_array_free(_G.preview_fce, _G.allocator);

    _G = {};
}

CETECH_MODULE_DEF(
        asset_preview,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_world_a0);
            CETECH_GET_API(api, ct_entity_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_viewport_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_playground_a0);
        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            _shutdown();
        }
)