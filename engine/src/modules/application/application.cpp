//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>

#include <cetech/celib/allocator.h>
#include <cetech/celib/eventstream.inl>

#include <cetech/kernel/api_system.h>
#include <cetech/kernel/hash.h>
#include <cetech/kernel/config.h>
#include <cetech/modules/application.h>
#include <cetech/kernel/os.h>
#include <cetech/kernel/task.h>
#include <cetech/kernel/develop.h>

#include <cetech/modules/resource.h>
#include <cetech/modules/luasys.h>
#include <cetech/modules/renderer.h>
#include <cetech/modules/package.h>
#include <cetech/kernel/console_server.h>

#include "../../kernel/api_private.h"
#include "../../kernel/module_private.h"
#include "../../kernel/log_system_private.h"


#include <include/mpack/mpack.h>
#include <cetech/modules/input.h>

CETECH_DECL_API(ct_console_srv_a0);
CETECH_DECL_API(ct_develop_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_package_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_lua_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_window_a0);
CETECH_DECL_API(ct_time_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_machine_a0);
CETECH_DECL_API(ct_keyboard_a0);
CETECH_DECL_API(ct_mouse_a0);
CETECH_DECL_API(ct_gamepad_a0);

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

    const ct_game_callbacks *game;
    ct_window_t *main_window;
    int is_running;
    int init_error;
    float dt;
} _G;


//==============================================================================
// Private
//==============================================================================


static int _cmd_wait(mpack_node_t args,
                     mpack_writer_t *writer) {
    return 0;
}

//==============================================================================
// Interface
//==============================================================================

const char *application_platform();

const char *application_native_platform();

ct_window_t *application_get_main_window();


void application_quit() {
    _G.is_running = 0;
    _G.init_error = 0;
}

static ct_app_a0 a0 = {
        .quit = application_quit,
        .platform =  application_platform,
        .native_platform =  application_native_platform,
        .main_window =  application_get_main_window,
};

void _init_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_config_a0);
    CETECH_GET_API(api, ct_time_a0);
    CETECH_GET_API(api, ct_path_a0);
    CETECH_GET_API(api, ct_log_a0);
    CETECH_GET_API(api, ct_hash_a0);

    CETECH_GET_API(api, ct_console_srv_a0);
    CETECH_GET_API(api, ct_develop_a0);
    CETECH_GET_API(api, ct_renderer_a0);
    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_package_a0);
    CETECH_GET_API(api, ct_task_a0);
    CETECH_GET_API(api, ct_lua_a0);
    CETECH_GET_API(api, ct_window_a0);
    CETECH_GET_API(api, ct_machine_a0);

    CETECH_GET_API(api, ct_keyboard_a0);
    CETECH_GET_API(api, ct_mouse_a0);
    CETECH_GET_API(api, ct_gamepad_a0);
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


void application_register_api(struct ct_api_a0 *api) {
    api->register_api("ct_app_a0", &a0);
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

void application_start() {
    _G = {0};

    _init_api(api::v0());
    _init_config();

    ct_console_srv_a0.register_command("wait", _cmd_wait);

#if defined(CETECH_DEVELOP)
    ct_resource_a0.set_autoload(1);
#else
    ct_resource_a0.set_autoload(0);
#endif

#ifdef CETECH_CAN_COMPILE
    if (ct_config_a0.get_int(_G.config.compile)) {
        ct_resource_a0.compiler_compile_all();

        if (!ct_config_a0.get_int(_G.config.continue_)) {
            return;
        }
    }
#endif

    if (!ct_config_a0.get_int(_G.config.daemon)) {
        intptr_t wid = ct_config_a0.get_int(_G.config.wid);

        char title[128] = {0};
        snprintf(title, CETECH_ARRAY_LEN(title), "cetech - %s",
                 ct_config_a0.get_string(_G.config.boot_script));

        if (wid == 0) {
            _G.main_window = ct_window_a0.create(
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    ct_config_a0.get_int(_G.config.screen_x),
                    ct_config_a0.get_int(_G.config.screen_y),
                    ct_config_a0.get_int(_G.config.fullscreen)
                    ? WINDOW_FULLSCREEN : WINDOW_NOFLAG
            );
        } else {
            _G.main_window = ct_window_a0.create_from((void *) wid);
        }

        ct_renderer_a0.create(_G.main_window);
    }

    _boot_stage();

    uint64_t last_tick = ct_time_a0.perf_counter();
    _G.game = ct_lua_a0.get_game_callbacks();

    if (!_G.game->init()) {
        ct_log_a0.error(LOG_WHERE, "Could not init game.");
        return;
    };

    _G.is_running = 1;
    ct_log_a0.info("core.ready", "Run main loop");

    float lag = 0.0f;
    float frame_limit = 60.0f;
    float frame_time = (1.0f / frame_limit);
    float frame_time_accum = 0.0f;

    ct_console_srv_a0.push_begin();
    while (_G.is_running) {
        auto application_sd = ct_develop_a0.enter_scope(
                "Application:update()");

        uint64_t now_ticks = ct_time_a0.perf_counter();
        float dt =
                ((float) (now_ticks - last_tick)) / ct_time_a0.perf_freq();

        _G.dt = dt;
        last_tick = now_ticks;
        frame_time_accum += dt;

        ct_machine_a0.update();
        ct_keyboard_a0.update();
        ct_mouse_a0.update();
        ct_gamepad_a0.update();
        ct_console_srv_a0.update();

        _G.game->update(dt);

        if (frame_time_accum >= frame_time) {
            if (!ct_config_a0.get_int(_G.config.daemon)) {
                ct_scope_data render_sd = ct_develop_a0.enter_scope(
                        "Game:render()");
                _G.game->render();
                ct_develop_a0.leave_scope(render_sd);
            }

            frame_time_accum = 0.0f;
        }

        ct_develop_a0.leave_scope(application_sd);
        ct_develop_a0.push_record_float("engine.delta_time", dt);


        ct_develop_a0.after_update(dt);
        //thread_yield();
    }

    _G.game->shutdown();

    _boot_unload();
}

const char *application_native_platform() {
#if defined(CETECH_LINUX)
    return "linux";
#elif defined(CETECH_WINDOWS)
    return "windows";
#elif defined(CETECH_DARWIN)
    return "darwin";
#endif
    return NULL;
}

const char *application_platform() {
    return application_native_platform();
}

ct_window_t *application_get_main_window() {
    return _G.main_window;
}
