//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>

#include <cetech/celib/allocator.h>
#include <cetech/celib/eventstream.inl>

#include <cetech/kernel/api_system.h>
#include <cetech/kernel/hash.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/application.h>
#include <cetech/kernel/sdl2_os.h>
#include <cetech/kernel/task.h>
#include <cetech/kernel/develop.h>
#include <cetech/kernel/sdl2_machine.h>

#include <cetech/modules/resource.h>
#include <cetech/modules/luasys.h>
#include <cetech/modules/renderer.h>
#include <cetech/modules/package.h>
#include <cetech/modules/console_server.h>

#include "../api/api_private.h"
#include "../module/module_private.h"
#include "../log/log_system_private.h"


#include <include/mpack/mpack.h>
#include <cetech/modules/input.h>

CETECH_DECL_API(cnsole_srv_api_v0);
CETECH_DECL_API(develop_api_v0);
CETECH_DECL_API(renderer_api_v0);
CETECH_DECL_API(resource_api_v0);
CETECH_DECL_API(package_api_v0);
CETECH_DECL_API(task_api_v0);
CETECH_DECL_API(lua_api_v0);
CETECH_DECL_API(config_api_v0);
CETECH_DECL_API(os_window_api_v0);
CETECH_DECL_API(os_time_api_v0);
CETECH_DECL_API(os_path_v0);
CETECH_DECL_API(log_api_v0);
CETECH_DECL_API(hash_api_v0);
CETECH_DECL_API(api_v0);
CETECH_DECL_API(machine_api_v0);


CETECH_DECL_API(keyboard_api_v0);
CETECH_DECL_API(mouse_api_v0);
CETECH_DECL_API(gamepad_api_v0);



//==============================================================================
// Definess
//==============================================================================

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

struct GConfig {
    cvar_t boot_pkg;
    cvar_t boot_script;
    cvar_t screen_x;
    cvar_t screen_y;
    cvar_t fullscreen;

    cvar_t daemon;
    cvar_t compile;
    cvar_t continue_;
    cvar_t wait;
    cvar_t wid;
};

static struct ApplicationGlobals {
    struct GConfig config;

    const struct game_callbacks *game;
    os_window_t *main_window;
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

os_window_t *application_get_main_window();


void application_quit() {
    _G.is_running = 0;
    _G.init_error = 0;
}

static struct app_api_v0 api_v1 = {
        .quit = application_quit,
        .platform =  application_platform,
        .native_platform =  application_native_platform,
        .main_window =  application_get_main_window,
};

void _init_api(struct api_v0 *api) {
    CETECH_GET_API(api, config_api_v0);
    CETECH_GET_API(api, os_time_api_v0);
    CETECH_GET_API(api, os_path_v0);
    CETECH_GET_API(api, log_api_v0);
    CETECH_GET_API(api, hash_api_v0);

    CETECH_GET_API(api, cnsole_srv_api_v0);
    CETECH_GET_API(api, develop_api_v0);
    CETECH_GET_API(api, renderer_api_v0);
    CETECH_GET_API(api, resource_api_v0);
    CETECH_GET_API(api, package_api_v0);
    CETECH_GET_API(api, task_api_v0);
    CETECH_GET_API(api, lua_api_v0);
    CETECH_GET_API(api, os_window_api_v0);
    CETECH_GET_API(api, machine_api_v0);

    CETECH_GET_API(api, keyboard_api_v0);
    CETECH_GET_API(api, mouse_api_v0);
    CETECH_GET_API(api, gamepad_api_v0);
}

void _init_config() {
    _G.config = (struct GConfig) {
            .boot_pkg = config_api_v0.new_str("core.boot_pkg", "Boot package",
                                              "boot"),

            .boot_script = config_api_v0.new_str("core.boot_script",
                                                 "Boot script", "lua/boot"),

            .screen_x = config_api_v0.new_int("screen.x", "Screen width", 1024),
            .screen_y = config_api_v0.new_int("screen.y", "Screen height", 768),
            .fullscreen = config_api_v0.new_int("screen.fullscreen",
                                                "Fullscreen", 0),

            .daemon = config_api_v0.new_int("daemon", "Daemon mode", 0),
            .compile = config_api_v0.new_int("compile", "Comple", 0),
            .continue_ = config_api_v0.new_int("continue",
                                               "Continue after compile", 0),
            .wait = config_api_v0.new_int("wait", "Wait for client", 0),
            .wid = config_api_v0.new_int("wid", "Wid", 0)
    };
}


void application_register_api(struct api_v0 *api) {
    api->register_api("app_api_v0", &api_v1);
}

static void _boot_stage() {
    const char *boot_pkg_str = config_api_v0.get_string(_G.config.boot_pkg);
    uint64_t boot_pkg = hash_api_v0.id64_from_str(boot_pkg_str);
    uint64_t pkg = hash_api_v0.id64_from_str("package");

    uint64_t core_pkg = hash_api_v0.id64_from_str("core");
    uint64_t resources[] = {core_pkg, boot_pkg};

    resource_api_v0.load_now(pkg, resources, 2);

    package_api_v0.load(boot_pkg);
    package_api_v0.flush(boot_pkg);

    package_api_v0.load(core_pkg);
    package_api_v0.flush(core_pkg);

    uint64_t boot_script = hash_api_v0.id64_from_str(
            config_api_v0.get_string(_G.config.boot_script));
    lua_api_v0.execute_boot_script(boot_script);
}

static void _boot_unload() {
    uint64_t boot_pkg = hash_api_v0.id64_from_str(
            config_api_v0.get_string(_G.config.boot_pkg));
    uint64_t core_pkg = hash_api_v0.id64_from_str("core");
    uint64_t pkg = hash_api_v0.id64_from_str("package");

    uint64_t resources[] = {core_pkg, boot_pkg};

    package_api_v0.unload(boot_pkg);
    package_api_v0.unload(core_pkg);

    resource_api_v0.unload(pkg, resources, 2);
}

void application_start() {
    _G = {0};

    _init_api(api::v0());
    _init_config();

    cnsole_srv_api_v0.register_command("wait", _cmd_wait);

#if defined(CETECH_DEVELOP)
    resource_api_v0.set_autoload(1);
#else
    resource_api_v0.set_autoload(0);
#endif

#ifdef CETECH_CAN_COMPILE
    if (config_api_v0.get_int(_G.config.compile)) {
        resource_api_v0.compiler_compile_all();

        if (!config_api_v0.get_int(_G.config.continue_)) {
            return;
        }
    }
#endif

    if (!config_api_v0.get_int(_G.config.daemon)) {
        intptr_t wid = config_api_v0.get_int(_G.config.wid);

        char title[128] = {0};
        snprintf(title, CETECH_ARRAY_LEN(title), "cetech - %s",
                 config_api_v0.get_string(_G.config.boot_script));

        if (wid == 0) {
            _G.main_window = os_window_api_v0.create(
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    config_api_v0.get_int(_G.config.screen_x),
                    config_api_v0.get_int(_G.config.screen_y),
                    config_api_v0.get_int(_G.config.fullscreen)
                    ? WINDOW_FULLSCREEN : WINDOW_NOFLAG
            );
        } else {
            _G.main_window = os_window_api_v0.create_from((void *) wid);
        }

        renderer_api_v0.create(_G.main_window);
    }

    _boot_stage();

    uint64_t last_tick = os_time_api_v0.perf_counter();
    _G.game = lua_api_v0.get_game_callbacks();

    if (!_G.game->init()) {
        log_api_v0.error(LOG_WHERE, "Could not init game.");
        return;
    };

    _G.is_running = 1;
    log_api_v0.info("core.ready", "Run main loop");

    float lag = 0.0f;
    float frame_limit = 60.0f;
    float frame_time = (1.0f / frame_limit);
    float frame_time_accum = 0.0f;

    cnsole_srv_api_v0.push_begin();
    while (_G.is_running) {
        auto application_sd = develop_api_v0.enter_scope(
                "Application:update()");

        uint64_t now_ticks = os_time_api_v0.perf_counter();
        float dt =
                ((float) (now_ticks - last_tick)) / os_time_api_v0.perf_freq();

        _G.dt = dt;
        last_tick = now_ticks;
        frame_time_accum += dt;

        machine_api_v0.update();
        keyboard_api_v0.update();
        mouse_api_v0.update();
        gamepad_api_v0.update();
        cnsole_srv_api_v0.update();

        _G.game->update(dt);

        if (frame_time_accum >= frame_time) {
            if (!config_api_v0.get_int(_G.config.daemon)) {
                struct scope_data render_sd = develop_api_v0.enter_scope(
                        "Game:render()");
                _G.game->render();
                develop_api_v0.leave_scope(render_sd);
            }

            frame_time_accum = 0.0f;
        }

        develop_api_v0.leave_scope(application_sd);
        develop_api_v0.push_record_float("engine.delta_time", dt);


        develop_api_v0.after_update(dt);
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

os_window_t *application_get_main_window() {
    return _G.main_window;
}
