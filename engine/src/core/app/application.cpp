//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>

#include <cetech/celib/allocator.h>
#include <cetech/celib/eventstream.inl>

#include <cetech/core/api.h>
#include <cetech/core/hash.h>
#include <cetech/core/config.h>
#include <cetech/core/application.h>
#include <cetech/core/window.h>
#include <cetech/core/path.h>
#include <cetech/core/time.h>


#include <cetech/modules/task.h>
#include <cetech/modules/resource.h>
#include <cetech/modules/develop.h>
#include <cetech/modules/luasys.h>
#include <cetech/modules/renderer.h>
#include <cetech/modules/package.h>
#include <cetech/modules/console_server.h>

#include "../os/_os.h"
#include "../api/_api.h"
#include "../module/_module.h"
#include "../memory/_memory.h"
#include "../config/_config.h"
#include "../log/_log_system.h"

#include "static_module.h"

#include <include/mpack/mpack.h>

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
IMPORT_API(path_v0);
IMPORT_API(log_api_v0);
IMPORT_API(hash_api_v0);

//==============================================================================
// Definess
//==============================================================================

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

struct args {
    int argc;
    const char **argv;
};

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
    window_t main_window;
    struct args args;
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

void _init_api(struct api_v0 *api) {
    GET_API(api, cnsole_srv_api_v0);
    GET_API(api, develop_api_v0);
    GET_API(api, renderer_api_v0);
    GET_API(api, resource_api_v0);
    GET_API(api, package_api_v0);
    GET_API(api, task_api_v0);
    GET_API(api, lua_api_v0);
    GET_API(api, config_api_v0);
    GET_API(api, window_api_v0);
    GET_API(api, time_api_v0);
    GET_API(api, path_v0);
    GET_API(api, log_api_v0);
    GET_API(api, hash_api_v0);
}

int _init_config(struct api_v0 *api) {
    struct config_api_v0 config = *(struct config_api_v0 *) api->first(
            "config_api_v0").api;

    _G.config = (struct GConfig) {
            .boot_pkg = config.new_str("core.boot_pkg", "Boot package",
                                       "boot"),

            .boot_script = config.new_str("core.boot_script",
                                          "Boot script", "lua/boot"),

            .screen_x = config.new_int("screen.x", "Screen width", 1024),
            .screen_y = config.new_int("screen.y", "Screen height", 768),
            .fullscreen = config.new_int("screen.fullscreen",
                                         "Fullscreen", 0),

            .daemon = config.new_int("daemon", "Daemon mode", 0),
            .compile = config.new_int("compile", "Comple", 0),
            .continue_ = config.new_int("continue",
                                        "Continue after compile", 0),
            .wait = config.new_int("wait", "Wait for client", 0),
            .wid = config.new_int("wid", "Wid", 0)
    };

    // Cvar stage

    config.parse_core_args(_G.args.argc, _G.args.argv);

#ifdef CETECH_CAN_COMPILE
    if (config.get_int(_G.config.compile)) {
        char build_dir_full[1024] = {0};
        cvar_t bd = config.find("build");

        const char *build_dir_str = config.get_string(bd);
        path_v0.join(build_dir_full, 1024, build_dir_str,
                     application_platform());
        path_v0.make_path(build_dir_full);
        config.compile_global(&api_v1);
    }
#endif

    config.load_global(&api_v1);

    if (!config.parse_args(_G.args.argc, _G.args.argv)) {
        return 0;
    }

    config.log_all();

    return 1;
}

extern void error_init(struct api_v0 *api);

extern "C" void _init_core_modules() {
    ADD_STATIC_PLUGIN(os);
    ADD_STATIC_PLUGIN(blob);
    ADD_STATIC_PLUGIN(machine);
    ADD_STATIC_PLUGIN(task);
    ADD_STATIC_PLUGIN(developsystem);
    ADD_STATIC_PLUGIN(consoleserver);

    ADD_STATIC_PLUGIN(filesystem);
    ADD_STATIC_PLUGIN(resourcesystem);

#ifdef CETECH_CAN_COMPILE
    ADD_STATIC_PLUGIN(resourcecompiler);
#endif
}

int application_init(int argc,
                     const char **argv) {
    _G = {0};

    _G.args = (struct args) {.argc = argc, .argv = argv};

    log::init();
    memory::memsys_init(4 * 1024 * 1024);
    api::init(memory::_memsys_main_allocator());

    api_v0 *api = api::v0();

    log::register_api(api);
    error_init(api);

    api->register_api("app_api_v0", &api_v1);

    memory::memsys_init_api(api);
    os::init(api);

    module::init(memory::_memsys_main_allocator(), api);
    config::init(api);

    log::logdb_init_db(".", api);

    _init_core_modules();
    _init_static_modules();

    module::call_init_api();
    _init_api(api);

    module::load_dirs("./bin");

    module::call_init_cvar();

    if (!_init_config(api)) {
        return 0;
    };

    module::call_init();

    log_api_v0.set_wid_clb(task_api_v0.worker_id);

    cnsole_srv_api_v0.register_command("wait", _cmd_wait);

    return 1;
}

int application_shutdown() {
    log_api_v0.debug(LOG_WHERE, "Shutdown");

    module::call_shutdown();

    config::shutdown();
    module::shutdown();
    api::shutdown();
    memory::memsys_shutdown();
    log::shutdown();

    return !_G.init_error;
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

    uint64_t last_tick = time_api_v0.perf_counter();
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
        struct scope_data application_sd = develop_api_v0.enter_scope(
                "Application:update()");

        uint64_t now_ticks = time_api_v0.perf_counter();
        float dt =
                ((float) (now_ticks - last_tick)) / time_api_v0.perf_freq();

        _G.dt = dt;
        last_tick = now_ticks;
        frame_time_accum += dt;

        module::call_update();
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

        module::call_after_update(dt);
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

int main(int argc,
         const char **argv) {

    if (application_init(argc, argv)) {
        application_start();
    }

    return application_shutdown();
}
