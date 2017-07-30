//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>


#include <cetech/engine/application/application.h>
#include <cetech/engine/entity/entity.h>
#include <cetech/core/api/api_system.h>
#include <cetech/engine/resource/package.h>
#include <cetech/core/task/task.h>
#include <cetech/modules/luasys/luasys.h>
#include <cetech/core/config/config.h>
#include <cetech/core/os/time.h>
#include <cetech/core/os/path.h>
#include <cetech/core/log/log.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/memory/memory.h>
#include <cetech/engine/machine/machine.h>
#include <cetech/engine/resource/resource.h>

#include <include/mpack/mpack.h>
#include <celib/macros.h>
#include <cetech/core/module/module.h>
#include <celib/container_types.inl>
#include <celib/array.inl>

CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_package_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_time_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_memory_a0);

CETECH_DECL_API(ct_lua_a0);

//==============================================================================
// Definess
//==============================================================================

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

struct GConfig {
    ct_cvar boot_pkg;
    ct_cvar boot_script;
    ct_cvar screen_x;
    ct_cvar screen_y;
    ct_cvar fullscreen;

    ct_cvar daemon;
    ct_cvar compile;
    ct_cvar continue_;
    ct_cvar wait;
    ct_cvar wid;
};

static struct ApplicationGlobals {
    struct GConfig config;
    celib::Array<ct_app_on_update> on_update;

    const ct_game_callbacks *game;

    int is_running;
    int init_error;
    float dt;
} _G;


//==============================================================================
// Private
//==============================================================================


static int _cmd_wait(mpack_node_t args,
                     mpack_writer_t *writer) {
    CEL_UNUSED(args);
    CEL_UNUSED(writer);
    return 0;
}

//==============================================================================
// Interface
//==============================================================================

void application_quit() {
    _G.is_running = 0;
    _G.init_error = 0;
}

void _init_config() {
    _G.config = (struct GConfig) {
            .boot_pkg = ct_config_a0.new_str("core.boot_pkg", "Boot package",
                                             "boot"),

            .boot_script = ct_config_a0.new_str("core.boot_script",
                                                "Boot script", "lua/boot"),

            .screen_x = ct_config_a0.new_int("screen.x", "Screen width", 1024),
            .screen_y = ct_config_a0.new_int("screen.y", "Screen height", 768),
            .fullscreen = ct_config_a0.new_int("screen.fullscreen",
                                               "Fullscreen", 0),

            .daemon = ct_config_a0.new_int("daemon", "Daemon mode", 0),
            .compile = ct_config_a0.new_int("compile", "Comple", 0),
            .continue_ = ct_config_a0.new_int("continue",
                                              "Continue after compile", 0),
            .wait = ct_config_a0.new_int("wait", "Wait for client", 0),
            .wid = ct_config_a0.new_int("wid", "Wid", 0)
    };
}

static void _boot_stage() {
    const char *boot_pkg_str = ct_config_a0.get_string(_G.config.boot_pkg);
    uint64_t boot_pkg = ct_hash_a0.id64_from_str(boot_pkg_str);
    uint64_t pkg = ct_hash_a0.id64_from_str("package");

    uint64_t core_pkg = ct_hash_a0.id64_from_str("core");
    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_resource_a0.load_now(pkg, resources, 2);

    ct_package_a0.load(boot_pkg);
    ct_package_a0.flush(boot_pkg);

    ct_package_a0.load(core_pkg);
    ct_package_a0.flush(core_pkg);

    uint64_t boot_script = ct_hash_a0.id64_from_str(
            ct_config_a0.get_string(_G.config.boot_script));
    ct_lua_a0.execute_boot_script(boot_script);
}

static void _boot_unload() {
    uint64_t boot_pkg = ct_hash_a0.id64_from_str(
            ct_config_a0.get_string(_G.config.boot_pkg));
    uint64_t core_pkg = ct_hash_a0.id64_from_str("core");
    uint64_t pkg = ct_hash_a0.id64_from_str("package");

    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_package_a0.unload(boot_pkg);
    ct_package_a0.unload(core_pkg);

    ct_resource_a0.unload(pkg, resources, 2);
}


extern "C" void application_start() {
    _init_config();

#ifdef CETECH_CAN_COMPILE
    if (ct_config_a0.get_int(_G.config.compile)) {
        ct_resource_a0.compiler_compile_all();

        if (!ct_config_a0.get_int(_G.config.continue_)) {
            return;
        }
    }
#endif

    _boot_stage();

    uint64_t last_tick = ct_time_a0.perf_counter();
    _G.game = ct_lua_a0.get_game_callbacks();

    if (!_G.game->init()) {
        ct_log_a0.error(LOG_WHERE, "Could not init game.");
        return;
    };

    _G.is_running = 1;
    ct_log_a0.info("core.ready", "Run main loop");

    //float lag = 0.0f;
    float frame_limit = 60.0f;
    float frame_time = (1.0f / frame_limit);
    float frame_time_accum = 0.0f;

    while (_G.is_running) {

        uint64_t now_ticks = ct_time_a0.perf_counter();
        float dt =
                ((float) (now_ticks - last_tick)) / ct_time_a0.perf_freq();

        _G.dt = dt;
        last_tick = now_ticks;
        frame_time_accum += dt;

        for(uint32_t i  = 0; i < celib::array::size(_G.on_update); ++i) {
            _G.on_update[i](dt);
        }

        _G.game->update(dt);

        if (frame_time_accum >= frame_time) {
            if (!ct_config_a0.get_int(_G.config.daemon)) {
                _G.game->render();
            }

            frame_time_accum = 0.0f;
        }

        //thread_yield();
    }

    _G.game->shutdown();

    _boot_unload();
}

void register_on_update(ct_app_on_update on_update) {
    celib::array::push_back(_G.on_update, on_update);
}

void unregister_on_update(ct_app_on_update on_update) {
    const auto size = celib::array::size(_G.on_update);

    for(uint32_t i = 0; i < size; ++i) {
        if(_G.on_update[i] != on_update) {
            continue;
        }

        uint32_t last_idx = size - 1;
        _G.on_update[i] = _G.on_update[last_idx];

        celib::array::pop_back(_G.on_update);
        break;
    }
}

static ct_app_a0 a0 = {
        .quit = application_quit,
        .start = application_start,
        .register_on_update = register_on_update,
        .unregister_on_update = unregister_on_update
};

void app_init(ct_api_a0 *api) {
    api->register_api("ct_app_a0", &a0);

#if defined(CETECH_DEVELOP)
    ct_resource_a0.set_autoload(1);
#else
    ct_resource_a0.set_autoload(0);
#endif
    _G.on_update.init(ct_memory_a0.main_allocator());
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
            CETECH_GET_API(api, ct_lua_a0);
        },
        {
            app_init(api);
        },
        {
            CEL_UNUSED(api);
            _G.on_update.destroy();
        }
)