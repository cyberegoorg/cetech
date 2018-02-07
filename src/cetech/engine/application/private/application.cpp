//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>

#include <cetech/core/macros.h>
#include <cetech/core/containers/container_types.inl>

#include <cetech/core/containers/map.inl>

#include <cetech/engine/application/application.h>
#include <cetech/core/os/watchdog.h>
#include <cetech/core/fs/fs.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/core/api/api_system.h>
#include <cetech/engine/resource/package.h>
#include <cetech/core/task/task.h>
#include <cetech/core/config/config.h>
#include <cetech/core/os/time.h>
#include <cetech/core/os/path.h>
#include <cetech/core/log/log.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/memory/memory.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/core/module/module.h>
#include <cetech/engine/kernel/kernel.h>

// TODO: shit , app == module?
#include <cetech/engine/machine/machine.h>
#include <cetech/engine/renderer/renderer.h>

#include <cetech/core/containers/array.h>
#include <cetech/core/containers/hash.h>
#include <cetech/engine/world/world.h>

CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_package_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_time_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_module_a0);
CETECH_DECL_API(ct_watchdog_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_fs_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_machine_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_world_a0);

//==============================================================================
// Definess
//==============================================================================

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

static struct ApplicationGlobals {
    ct_cdb_obj_t *config_object;

    ct_app_on_init *on_init;
    ct_app_on_shutdown *on_shutdown;
    ct_app_on_update *on_update;

    ct_hash_t game_map;
    ct_game_fce *game;

    ct_game_fce active_game;
    int is_running;
    ct_alloc *allocator;
} _G;


//==============================================================================
// Interface
//==============================================================================

void application_quit() {
    _G.is_running = 0;
}

#define CONFIG_BOOT_PKG CT_ID64_0(CONFIG_BOOT_PKG_ID)
#define CONFIG_DAEMON CT_ID64_0(CONFIG_DAEMON_ID)
#define CONFIG_COMPILE CT_ID64_0(CONFIG_COMPILE_ID)
#define CONFIG_CONTINUE CT_ID64_0(CONFIG_CONTINUE_ID)
#define CONFIG_WAIT CT_ID64_0(CONFIG_WAIT_ID)
#define CONFIG_GAME CT_ID64_0(CONFIG_GAME_ID)

void _init_config() {
    _G.config_object = ct_config_a0.config_object();

    ct_cdb_writer_t *writer = ct_cdb_a0.write_begin(_G.config_object);

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_BOOT_PKG)) {
        ct_cdb_a0.set_string(writer, CONFIG_BOOT_PKG, "boot");
    }

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_GAME)) {
        ct_cdb_a0.set_string(writer, CONFIG_GAME, "playground");
    }

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_DAEMON)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_DAEMON, 0);
    }

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_COMPILE)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_COMPILE, 0);
    }

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_CONTINUE)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_CONTINUE, 0);
    }

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_WAIT)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_WAIT, 0);
    }

    ct_cdb_a0.write_commit(writer);
}

static void _boot_stage() {
    const char *boot_pkg_str = ct_cdb_a0.read_str(_G.config_object,
                                                  CONFIG_BOOT_PKG, "");
    uint64_t boot_pkg = CT_ID64_0(boot_pkg_str);
    uint64_t pkg = CT_ID64_0("package");

    uint64_t core_pkg = CT_ID64_0("core/core");
    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_resource_a0.load_now(pkg, resources, 2);

    ct_task_counter_t *boot_pkg_cnt = ct_package_a0.load(boot_pkg);
    ct_package_a0.flush(boot_pkg_cnt);

    ct_task_counter_t *core_pkg_cnt = ct_package_a0.load(core_pkg);
    ct_package_a0.flush(core_pkg_cnt);
}

static void _boot_unload() {
    const char *boot_pkg_str = ct_cdb_a0.read_str(_G.config_object,
                                                  CONFIG_BOOT_PKG, "");
    uint64_t boot_pkg = CT_ID64_0(boot_pkg_str);

    uint64_t core_pkg = CT_ID64_0("core/core");
    uint64_t pkg = CT_ID64_0("package");

    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_package_a0.unload(boot_pkg);
    ct_package_a0.unload(core_pkg);

    ct_resource_a0.unload(pkg, resources, 2);
}

void set_active_game(uint64_t name) {
    uint64_t idx = ct_hash_lookup(&_G.game_map, name, UINT64_MAX);
    if (idx != UINT64_MAX) {
        _G.active_game = _G.game[idx];
    }
}

static void check_machine() {
    ct_event_header *event = ct_machine_a0.event_begin();

    while (event != ct_machine_a0.event_end()) {
        switch (event->type) {
            case EVENT_QUIT:
                application_quit();
                break;

            default:
                break;
        }

        event = ct_machine_a0.event_next(event);
    }
}

void simplesimu(struct ct_entity *ent,
                struct ct_cdb_obj_t **obj,
                uint32_t n,
                float dt) {

}

extern "C" void application_start() {
    _init_config();

    if (ct_cdb_a0.read_uint32(_G.config_object, CONFIG_COMPILE, 0)) {
        ct_resource_a0.compiler_compile_all();

        if (!ct_cdb_a0.read_uint32(_G.config_object, CONFIG_CONTINUE, 0)) {
            return;
        }
    }

    _boot_stage();

    uint64_t last_tick = ct_time_a0.perf_counter();

    for (uint32_t i = 0; i < ct_array_size(_G.on_init); ++i) {
        _G.on_init[i]();
    }

    set_active_game(CT_ID64_0(ct_cdb_a0.read_str(_G.config_object,
                                                 CONFIG_GAME, "")));

    if (_G.active_game.on_init) {
        _G.active_game.on_init();
    }

    uint64_t fq = ct_time_a0.perf_freq();

//    ct_world_a0.add_simulation(
//            ct_world_a0.component_mask(CT_ID64_0("transformation")),
//            simplesimu);

    _G.is_running = 1;
    while (_G.is_running) {
        uint64_t now_ticks = ct_time_a0.perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / fq;
        last_tick = now_ticks;

        ct_fs_a0.check_wd();
        ct_ydb_a0.check_fs();
        ct_resource_a0.compiler_check_fs();

#if CETECH_DEVELOP
        ct_module_a0.check_modules(); // TODO: SHIT...
#endif
        ct_machine_a0.update(dt);
        check_machine();

        for (uint32_t i = 0; i < ct_array_size(_G.on_update); ++i) {
            _G.on_update[i](dt);
        }

        if (_G.active_game.on_update) {
            _G.active_game.on_update(dt);
        }

        if (!ct_cdb_a0.read_uint32(_G.config_object, CONFIG_DAEMON, 0)) {
            ct_renderer_a0.render(_G.active_game.on_render ?
                                  _G.active_game.on_render : NULL);
        }
//
//        sleep(0);
    }

    if (_G.active_game.on_shutdown) {
        _G.active_game.on_shutdown();
    }

    for (uint32_t i = 0; i < ct_array_size(_G.on_shutdown); ++i) {
        _G.on_shutdown[i]();
    }

    _boot_unload();
}

#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                            \
        ct_array_push(_G.name, name, _G.allocator);                            \
    }                                                                          \
    static void unregister_## name ## _(type name) {                           \
        const auto size = ct_array_size(_G.name);                              \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            ct_array_pop_back(_G.name);                                        \
            break;                                                             \
        }                                                                      \
    }

_DEF_ON_CLB_FCE(ct_app_on_init, on_init)

_DEF_ON_CLB_FCE(ct_app_on_shutdown, on_shutdown)

_DEF_ON_CLB_FCE(ct_app_on_update, on_update)


#undef _DEF_ON_CLB_FCE

void register_game(uint64_t name,
                   ct_game_fce game) {
    ct_array_push(_G.game, game, _G.allocator);
    ct_hash_add(&_G.game_map, name, ct_array_size(_G.game) - 1, _G.allocator);
}

void unregister_game(uint64_t name) {
    ct_hash_remove(&_G.game_map, name);
}


static ct_app_a0 a0 = {
        .quit = application_quit,
        .start = application_start,

        .register_game = register_game,
        .unregister_game = unregister_game,
        .set_active_game = set_active_game,

        .register_on_init = register_on_init_,
        .unregister_on_init = unregister_on_init_,

        .register_on_shutdown = register_on_shutdown_,
        .unregister_on_shutdown = unregister_on_shutdown_,

        .register_on_update = register_on_update_,
        .unregister_on_update = unregister_on_update_,

};

void app_init(struct ct_api_a0 *api) {
    api->register_api("ct_app_a0", &a0);

    _G.allocator = ct_memory_a0.main_allocator();

#if defined(CETECH_DEVELOP)
    ct_resource_a0.set_autoload(true);
#else
    ct_resource_a0.set_autoload(0);
#endif
}

CETECH_MODULE_DEF(
        application,
        {
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_time_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hashlib_a0);

            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_package_a0);
            CETECH_GET_API(api, ct_task_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_module_a0);
            CETECH_GET_API(api, ct_watchdog_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_fs_a0);
            CETECH_GET_API(api, ct_ydb_a0);

            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_machine_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_world_a0);

            ct_api_a0 = *api;
        },
        {
            CT_UNUSED(reload);

            app_init(api);
        },
        {
            CT_UNUSED(api, reload);

            ct_array_free(_G.on_init, _G.allocator);
            ct_array_free(_G.on_shutdown, _G.allocator);
            ct_array_free(_G.on_update, _G.allocator);
            ct_array_free(_G.game, _G.allocator);
            ct_hash_free(&_G.game_map, _G.allocator);
        }
)
