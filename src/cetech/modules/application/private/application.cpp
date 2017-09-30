//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>

#include <celib/macros.h>
#include <celib/container_types.inl>
#include <celib/array.inl>
#include <celib/map.inl>

#include <cetech/modules/application/application.h>
#include <cetech/kernel/watchdog.h>
#include <cetech/kernel/filesystem.h>
#include <cetech/kernel/ydb.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/package.h>
#include <cetech/kernel/task.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/time.h>
#include <cetech/kernel/path.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/resource.h>
#include <cetech/kernel/module.h>

// TODO: shit , app == module?
#include <cetech/modules/machine/machine.h>
#include <cetech/modules/entity/entity.h>
#include <cetech/modules/input/input.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/renderer/renderer.h>

CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_package_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_time_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_module_a0);
CETECH_DECL_API(ct_watchdog_a0);
CETECH_DECL_API(ct_mouse_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_machine_a0);

//==============================================================================
// Definess
//==============================================================================

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

struct GConfig {
    ct_cvar boot_pkg;
    ct_cvar screen_x;
    ct_cvar screen_y;
    ct_cvar fullscreen;

    ct_cvar daemon;
    ct_cvar compile;
    ct_cvar continue_;
    ct_cvar wait;
    ct_cvar wid;
    ct_cvar game;
};

static struct ApplicationGlobals {
    struct GConfig config;
    celib::Array<ct_app_on_init> on_init;
    celib::Array<ct_app_on_shutdown> on_shutdown;
    celib::Array<ct_app_on_update> on_update;

    celib::Map<ct_game_fce> game_map;
    ct_game_fce active_game;
    int is_running;
} _G;


//==============================================================================
// Interface
//==============================================================================

void application_quit() {
    _G.is_running = 0;
}

void _init_config() {
    _G.config = (struct GConfig) {
            .boot_pkg = ct_config_a0.new_str("core.boot_pkg", "Boot package",
                                             "boot"),

            .daemon = ct_config_a0.new_int("daemon", "Daemon mode", 0),
            .compile = ct_config_a0.new_int("compile", "Comple", 0),
            .continue_ = ct_config_a0.new_int("continue",
                                              "Continue after compile", 0),
            .wait = ct_config_a0.new_int("wait", "Wait for client", 0),
            .game = ct_config_a0.new_str("game", "Boot game name", "playground"),
    };
}

static void _boot_stage() {
    const char *boot_pkg_str = ct_config_a0.get_string(_G.config.boot_pkg);
    uint64_t boot_pkg = CT_ID64_0(boot_pkg_str);
    uint64_t pkg = CT_ID64_0("package");

    uint64_t core_pkg = CT_ID64_0("core/core");
    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_resource_a0.load_now(pkg, resources, 2);

    ct_package_a0.load(boot_pkg);
    ct_package_a0.flush(boot_pkg);

    ct_package_a0.load(core_pkg);
    ct_package_a0.flush(core_pkg);
}

static void _boot_unload() {
    const char *boot_pkg_str = ct_config_a0.get_string(_G.config.boot_pkg);
    uint64_t boot_pkg = CT_ID64_0(boot_pkg_str);

    uint64_t core_pkg = CT_ID64_0("core/core");
    uint64_t pkg = CT_ID64_0("package");

    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_package_a0.unload(boot_pkg);
    ct_package_a0.unload(core_pkg);

    ct_resource_a0.unload(pkg, resources, 2);
}

void set_active_game(uint64_t name) {
    static ct_game_fce null_game = {};

    ct_game_fce game = celib::map::get(_G.game_map, name, {});
    if(!::memcmp(&game, &null_game, sizeof(ct_game_fce))){
        return;
    }

    _G.active_game = game;
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

extern "C" void application_start() {
    _init_config();

    if (ct_config_a0.get_int(_G.config.compile)) {
        ct_resource_a0.compiler_compile_all();

        if (!ct_config_a0.get_int(_G.config.continue_)) {
            return;
        }
    }

    _boot_stage();

    uint64_t last_tick = ct_time_a0.perf_counter();

    for (uint32_t i = 0; i < celib::array::size(_G.on_init); ++i) {
        _G.on_init[i]();
    }

    set_active_game(CT_ID64_0(ct_config_a0.get_string(_G.config.game)));

    if(_G.active_game.on_init) {
        _G.active_game.on_init();
    }

    _G.is_running = 1;
    while (_G.is_running) {
        uint64_t fq = ct_time_a0.perf_freq();
        uint64_t now_ticks = ct_time_a0.perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / fq;
        last_tick = now_ticks;

        ct_filesystem_a0.check_wd();
        ct_ydb_a0.check_fs();
        ct_resource_a0.compiler_check_fs();

#if CETECH_DEVELOP
        ct_module_a0.check_modules(); // TODO: SHIT...
#endif
        ct_machine_a0.update(dt);
        check_machine();

        for (uint32_t i = 0; i < celib::array::size(_G.on_update); ++i) {
            _G.on_update[i](dt);
        }

        if(_G.active_game.on_update) {
            _G.active_game.on_update(dt);
        }

        CETECH_GET_API(&ct_api_a0, ct_mouse_a0); // TODO: WTF

        if (!ct_config_a0.get_int(_G.config.daemon)) {
            ct_renderer_a0.render(_G.active_game.on_render? _G.active_game.on_render : NULL);
        }

        sleep(0);
    }

    if(_G.active_game.on_shutdown) {
        _G.active_game.on_shutdown();
    }

    for (uint32_t i = 0; i < celib::array::size(_G.on_shutdown); ++i) {
        _G.on_shutdown[i]();
    }

    _boot_unload();
}

#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                            \
        celib::array::push_back(_G.name, name);                                \
    }                                                                          \
    static void unregister_## name ## _(type name) {                           \
        const auto size = celib::array::size(_G.name);                         \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            celib::array::pop_back(_G.name);                                   \
            break;                                                             \
        }                                                                      \
    }

_DEF_ON_CLB_FCE(ct_app_on_init, on_init)

_DEF_ON_CLB_FCE(ct_app_on_shutdown, on_shutdown)

_DEF_ON_CLB_FCE(ct_app_on_update, on_update)


#undef _DEF_ON_CLB_FCE

void register_game(uint64_t name, ct_game_fce game) {
    celib::map::set(_G.game_map, name, game);
}

void unregister_game(uint64_t name) {
    celib::map::remove(_G.game_map, name);
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

#if defined(CETECH_DEVELOP)
    ct_resource_a0.set_autoload(1);
#else
    ct_resource_a0.set_autoload(0);
#endif

    _G.on_update.init(ct_memory_a0.main_allocator());
    _G.on_init.init(ct_memory_a0.main_allocator());
    _G.on_shutdown.init(ct_memory_a0.main_allocator());
    _G.game_map.init(ct_memory_a0.main_allocator());
}

CETECH_MODULE_DEF(
        application,
        {
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_time_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hash_a0);

            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_package_a0);
            CETECH_GET_API(api, ct_task_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_module_a0);
            CETECH_GET_API(api, ct_watchdog_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_ydb_a0);

            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_machine_a0);

            ct_api_a0 = *api;
        },
        {
            CEL_UNUSED(reload);

            app_init(api);
        },
        {
            CEL_UNUSED(api, reload);

            _G.on_init.destroy();
            _G.on_shutdown.destroy();
            _G.on_update.destroy();
            _G.game_map.destroy();
        }
)
