//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>

#include <cetech/core/allocator.h>

#include <cetech/core/os.h>
#include <cetech/core/window.h>
#include <cetech/core/hash.h>
#include <cetech/core/cmd_line.h>
#include <cetech/core/eventstream.inl>
#include <cetech/core/memory.h>

#include "app.h"
#include <cetech/kernel/config.h>
#include <cetech/kernel/application.h>
#include <cetech/kernel/resource.h>
#include <cetech/kernel/develop.h>
#include <cetech/kernel/task.h>

#include "../static_systems.h"

#include <cetech/modules/luasys/luasys.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/core/fs.h>

#define LOG_WHERE "application"


IMPORT_API(cnsole_srv_api_v0);
IMPORT_API(develop_api_v0);
IMPORT_API(renderer_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(package_api_v0);
IMPORT_API(task_api_v0);
IMPORT_API(lua_api_v0);
IMPORT_API(config_api_v0);
IMPORT_API(window_api_v0);
IMPORT_API(time_api_v0);

//==============================================================================
// Definess
//==============================================================================

#define _G ApplicationGlobals


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

struct G {
    struct GConfig config;

    const struct game_callbacks *game;
    window_t main_window;
    struct args args;
    int is_running;
    int init_error;
    float dt;
} _G = {0};


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

window_t application_get_main_window();

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

void _init_api() {
    INIT_API(module_get_engine_api, cnsole_srv_api_v0, CONSOLE_SERVER_API_ID);
    INIT_API(module_get_engine_api, develop_api_v0, DEVELOP_SERVER_API_ID);
    INIT_API(module_get_engine_api, renderer_api_v0, RENDERER_API_ID);
    INIT_API(module_get_engine_api, resource_api_v0, RESOURCE_API_ID);
    INIT_API(module_get_engine_api, package_api_v0, PACKAGE_API_ID);
    INIT_API(module_get_engine_api, task_api_v0, TASK_API_ID);
    INIT_API(module_get_engine_api, lua_api_v0, LUA_API_ID);
    INIT_API(module_get_engine_api, config_api_v0, CONFIG_API_ID);
    INIT_API(module_get_engine_api, window_api_v0, WINDOW_API_ID);
    INIT_API(module_get_engine_api, time_api_v0, TIME_API_ID);
}

int _init_config() {
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



    // Cvar stage

    config_api_v0.parse_core_args(_G.args.argc, _G.args.argv);
#ifdef CETECH_CAN_COMPILE
    if (config_api_v0.get_int(_G.config.compile)) {
        char build_dir_full[1024] = {0};
        resource_api_v0.compiler_get_build_dir(build_dir_full,
                                               CETECH_ARRAY_LEN(build_dir_full),
                                               application_platform());

        dir_make_path(build_dir_full);

        config_api_v0.compile_global();
    }
#endif

    config_api_v0.load_global();

    if (!config_api_v0.parse_args(_G.args.argc, _G.args.argv)) {
        return 0;
    }

    config_api_v0.log_all();

    return 1;
}

int application_init(int argc,
                     const char **argv) {
    _G = (struct G) {0};
    _G.args = (struct args) {.argc = argc, .argv = argv};

    log_init();
    log_register_handler(log_stdout_handler, NULL);
    logdb_init_db(".");

    log_debug(LOG_WHERE, "Init (global size: %lu)", sizeof(struct G));

    memsys_init(4 * 1024 * 1024);

    ADD_STATIC_PLUGIN(memsys);
    ADD_STATIC_PLUGIN(config);
    ADD_STATIC_PLUGIN(handler);
    ADD_STATIC_PLUGIN(application);

    cvar_init();
    module_call_init_cvar();
    module_call_init();

    ADD_STATIC_PLUGIN(sdl);
    ADD_STATIC_PLUGIN(machine);
    ADD_STATIC_PLUGIN(task);
    ADD_STATIC_PLUGIN(consoleserver);
    ADD_STATIC_PLUGIN(developsystem);

    ADD_STATIC_PLUGIN(filesystem);
    ADD_STATIC_PLUGIN(resourcesystem);

#ifdef CETECH_CAN_COMPILE
    ADD_STATIC_PLUGIN(resourcecompiler);
#endif

    _init_static_modules();

    module_load_dirs("./bin");
    module_call_init_cvar();

    _init_api();

    if (!_init_config()) {
        return 0;
    };

    module_call_init();

    log_set_wid_clb(task_api_v0.worker_id);

    cnsole_srv_api_v0.consolesrv_register_command("wait", _cmd_wait);

    return 1;
}

int application_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    module_call_shutdown();

    cvar_shutdown();
    memsys_shutdown();
    log_shutdown();

    return !_G.init_error;
}

static void _boot_stage() {
    const char *boot_pkg_str = config_api_v0.get_string(_G.config.boot_pkg);
    stringid64_t boot_pkg = stringid64_from_string(boot_pkg_str);
    stringid64_t pkg = stringid64_from_string("package");

    stringid64_t core_pkg = stringid64_from_string("core");
    stringid64_t resources[] = {core_pkg, boot_pkg};

    resource_api_v0.load_now(pkg, resources, 2);

    package_api_v0.load(core_pkg);
    package_api_v0.flush(core_pkg);
    package_api_v0.load(boot_pkg);
    package_api_v0.flush(boot_pkg);


    stringid64_t boot_script = stringid64_from_string(
            config_api_v0.get_string(_G.config.boot_script));
    lua_api_v0.execute_boot_script(boot_script);
}


static void _boot_unload() {
    stringid64_t boot_pkg = stringid64_from_string(
            config_api_v0.get_string(_G.config.boot_pkg));
    stringid64_t core_pkg = stringid64_from_string("core");
    stringid64_t pkg = stringid64_from_string("package");

    stringid64_t resources[] = {core_pkg, boot_pkg};

    package_api_v0.unload(boot_pkg);
    package_api_v0.unload(core_pkg);

    resource_api_v0.unload(pkg, resources, 2);
}

void application_start() {
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
            _G.main_window = window_api_v0.create(
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    config_api_v0.get_int(_G.config.screen_x),
                    config_api_v0.get_int(_G.config.screen_y),
                    config_api_v0.get_int(_G.config.fullscreen)
                    ? WINDOW_FULLSCREEN : WINDOW_NOFLAG
            );
        } else {
            _G.main_window = window_api_v0.create_from((void *) wid);
        }

        renderer_api_v0.create(_G.main_window);
    }

    _boot_stage();

    uint64_t last_tick = time_api_v0.get_perf_counter();
    _G.game = lua_api_v0.get_game_callbacks();

    if (!_G.game->init()) {
        log_error(LOG_WHERE, "Could not init game.");
        return;
    };

    _G.is_running = 1;
    log_info("core.ready", "Run main loop");

    float lag = 0.0f;
    float frame_limit = 60.0f;
    float frame_time = (1.0f / frame_limit);
    float frame_time_accum = 0.0f;

    cnsole_srv_api_v0.consolesrv_push_begin();
    while (_G.is_running) {
        struct scope_data application_sd = develop_api_v0.enter_scope(
                "Application:update()");

        uint64_t now_ticks = time_api_v0.get_perf_counter();
        float dt =
                ((float) (now_ticks - last_tick)) / time_api_v0.get_perf_freq();

        _G.dt = dt;
        last_tick = now_ticks;
        frame_time_accum += dt;

        module_call_update();
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

        module_call_after_update(dt);
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

window_t application_get_main_window() {
    return _G.main_window;
}


void *application_get_module_api(int api) {

    if (api == PLUGIN_EXPORT_API_ID) {
        static struct module_api_v0 module = {0};
        return &module;
    } else if (api == APPLICATION_API_ID) {
        return &api_v1;
    }

    return 0;
}