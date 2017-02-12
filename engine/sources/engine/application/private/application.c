//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>

#include <celib/time/time.h>
#include <celib/window/types.h>
#include <celib/window/window.h>
#include <celib/string/stringid.h>
#include <celib/os/cmd_line.h>

#include <engine/application/application.h>
#include <engine/config/cvar.h>
#include <celib/machine/machine.h>
#include <engine/resource/resource.h>
#include <engine/luasys/luasys.h>
#include <engine/renderer/renderer.h>
#include <engine/develop/develop_system.h>
#include <celib/thread/thread.h>
#include <engine/plugin/plugin.h>
#include <engine/memory/memsys.h>
#include <engine/static_systems.h>
#include <engine/input/keyboard.h>
#include <engine/input/mouse.h>
#include <engine/input/gamepad.h>

#include "engine/develop/console_server.h"
#include "engine/task/task.h"

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

#define _G ApplicationGlobals

static struct G {
    struct {
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
    } config;

    const struct game_callbacks *game;
    cel_window_t main_window;
    struct args args;
    int is_running;
    int init_error;
    float dt;
} ApplicationGlobals = {0};


//==============================================================================
// Private
//==============================================================================

static char _get_worker_id() {
    return 0;
}

//==============================================================================
// Interface
//==============================================================================

void application_quit() {
    _G.is_running = 0;
    _G.init_error = 0;
}

static int _cmd_wait(mpack_node_t args,
                     mpack_writer_t *writer) {
    return 0;
}

static struct ConsoleServerApiV1 ConsoleServerApiV1;
static struct DevelopSystemApiV1 DevelopSystemApiV1;
static struct RendererApiV1 RendererApiV1;
static struct ResourceApiV1 ResourceApiV1;
static struct PackageApiV1 PackageApiV1;
static struct TaskApiV1 TaskApiV1;

int application_init(int argc,
                     const char **argv) {
    _G = (struct G) {0};
    _G.args = (struct args) {.argc = argc, .argv = argv};

    log_init(_get_worker_id);
    log_register_handler(log_stdout_handler, NULL);
    logdb_init_db(".");

    log_debug(LOG_WHERE, "Init (global size: %lu)", sizeof(struct G));

    memsys_init(4 * 1024 * 1024);
    _init_static_plugins();

    cvar_init();

    _G.config.boot_pkg = cvar_new_str("core.boot_pkg", "Boot package", "boot");
    _G.config.boot_script = cvar_new_str("core.boot_script", "Boot script", "lua/boot");

    _G.config.screen_x = cvar_new_int("screen.x", "Screen width", 1024);
    _G.config.screen_y = cvar_new_int("screen.y", "Screen height", 768);
    _G.config.fullscreen = cvar_new_int("screen.fullscreen", "Fullscreen", 0);

    _G.config.daemon = cvar_new_int("daemon", "Daemon mode", 0);
    _G.config.compile = cvar_new_int("compile", "Comple", 0);
    _G.config.continue_ = cvar_new_int("continue", "Continue after compile", 0);
    _G.config.wait = cvar_new_int("wait", "Wait for client", 0);
    _G.config.wid = cvar_new_int("wid", "Wid", 0);


    plugin_load_dirs("./bin");
    plugin_call_init_cvar();
    machine_init(0);

    ConsoleServerApiV1 = *((struct ConsoleServerApiV1 *) plugin_get_engine_api(CONSOLE_SERVER_API_ID, 0));
    DevelopSystemApiV1 = *((struct DevelopSystemApiV1 *) plugin_get_engine_api(DEVELOP_SERVER_API_ID, 0));
    RendererApiV1 = *((struct RendererApiV1 *) plugin_get_engine_api(RENDERER_API_ID, 0));
    ResourceApiV1 = *((struct ResourceApiV1 *) plugin_get_engine_api(RESOURCE_API_ID, 0));
    PackageApiV1 = *((struct PackageApiV1 *) plugin_get_engine_api(PACKAGE_API_ID, 0));
    TaskApiV1 = *((struct TaskApiV1 *) plugin_get_engine_api(TASK_API_ID, 0));

    // Cvar stage

    cvar_parse_core_args(_G.args);
    if (cvar_get_int(_G.config.compile)) {
        ResourceApiV1.compiler_create_build_dir();
        cvar_compile_global();
    }

    cvar_load_global();

    if (!cvar_parse_args(_G.args)) {
        return 0;
    }

    cvar_log_all();

    plugin_call_init();
    machine_init(1);


    log_set_wid_clb(TaskApiV1.worker_id);

    ConsoleServerApiV1.consolesrv_register_command("wait", _cmd_wait);

    return 1;
}

int application_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    plugin_call_shutdown();
    machine_shutdown();
    cvar_shutdown();
    memsys_shutdown();
    log_shutdown();

    return !_G.init_error;
}

static void _boot_stage() {
    stringid64_t boot_pkg = stringid64_from_string(cvar_get_string(_G.config.boot_pkg));
    stringid64_t pkg = stringid64_from_string("package");

    // TODO: remove, this must be done by boot_package and load in boot_script
    //if (!cvar_get_int(_G.config.daemon)) {
    stringid64_t core_pkg = stringid64_from_string("core");
    ResourceApiV1.load_now(pkg, &core_pkg, 1);
    PackageApiV1.load(core_pkg);
    PackageApiV1.flush(core_pkg);
    //}

    ResourceApiV1.load_now(pkg, &boot_pkg, 1);
    PackageApiV1.load(boot_pkg);
    PackageApiV1.flush(boot_pkg);

    stringid64_t boot_script = stringid64_from_string(cvar_get_string(_G.config.boot_script));
    luasys_execute_boot_script(boot_script);
}


static void _boot_unload() {
    stringid64_t boot_pkg = stringid64_from_string(cvar_get_string(_G.config.boot_pkg));
    stringid64_t pkg = stringid64_from_string("package");

    PackageApiV1.unload(boot_pkg);
    ResourceApiV1.unload(pkg, &boot_pkg, 1);

    //if (!cvar_get_int(_G.config.daemon)) {
    stringid64_t core_pkg = stringid64_from_string("core");
    ResourceApiV1.load_now(pkg, &core_pkg, 1);
    PackageApiV1.load(core_pkg);
    PackageApiV1.flush(core_pkg);
    //}

}

void application_start() {
#if defined(CETECH_DEVELOP)
    ResourceApiV1.set_autoload(1);
#else
    ResourceApiV1.set_autoload(0);
#endif

    if (cvar_get_int(_G.config.compile)) {
        ResourceApiV1.compiler_compile_all();

        if (!cvar_get_int(_G.config.continue_)) {
            return;
        }
    }

    if (!cvar_get_int(_G.config.daemon)) {
        intptr_t wid = cvar_get_int(_G.config.wid);

        char title[128] = {0};
        snprintf(title, CEL_ARRAY_LEN(title), "cetech - %s", cvar_get_string(_G.config.boot_script));

        if (wid == 0) {
            _G.main_window = cel_window_new(
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    cvar_get_int(_G.config.screen_x), cvar_get_int(_G.config.screen_y),
                    cvar_get_int(_G.config.fullscreen) ? WINDOW_FULLSCREEN : WINDOW_NOFLAG
            );
        } else {
            _G.main_window = cel_window_new_from((void *) wid);
        }

        RendererApiV1.create(_G.main_window);
    }

    _boot_stage();

    u64 last_tick = cel_get_perf_counter();
    _G.game = luasys_get_game_callbacks();

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

    ConsoleServerApiV1.consolesrv_push_begin();
    while (_G.is_running) {
        struct scope_data application_sd = DevelopSystemApiV1.enter_scope("Application:update()");

        u64 now_ticks = cel_get_perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / cel_get_perf_freq();

        _G.dt = dt;
        last_tick = now_ticks;
        frame_time_accum += dt;


        machine_process();

        plugin_call_update();
        _G.game->update(dt);

        if (frame_time_accum >= frame_time) {
            if (!cvar_get_int(_G.config.daemon)) {
                struct scope_data render_sd = DevelopSystemApiV1.enter_scope("Game::render()");
                _G.game->render();
                DevelopSystemApiV1.leave_scope("Game::render()", render_sd);
            }

            frame_time_accum = 0.0f;
        }

        DevelopSystemApiV1.leave_scope("Application:update()", application_sd);
        DevelopSystemApiV1.push_record_float("engine.delta_time", dt);

        plugin_call_after_update(dt);
        //cel_thread_yield();
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

cel_window_t application_get_main_window() {
    return _G.main_window;
}
