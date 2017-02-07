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


int application_init(int argc,
                     const char **argv) {
    _G = (struct G) {0};
    _G.args = (struct args) {.argc = argc, .argv = argv};

    log_init(_get_worker_id);
    log_register_handler(log_stdout_handler, NULL);
    logdb_init_db(".");

    log_debug(LOG_WHERE, "Init (global size: %lu)", sizeof(struct G));

    memsys_init(4 * 1024 * 1024);
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


    _init_static_plugins();

    plugin_load_dirs("./bin");
    plugin_call_init_cvar();
    machine_init(0);

    // Cvar stage

    cvar_parse_core_args(_G.args);
    if (cvar_get_int(_G.config.compile)) {
        resource_compiler_create_build_dir();
        cvar_compile_global();
    }

    cvar_load_global();

    if (!cvar_parse_args(_G.args)) {
        return 0;
    }

    cvar_log_all();

    plugin_call_init();
    machine_init(1);


    log_set_wid_clb(taskmanager_worker_id);

    consolesrv_register_command("wait", _cmd_wait);

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
        resource_load_now(pkg, &core_pkg, 1);
        package_load(core_pkg);
        package_flush(core_pkg);
    //}

    resource_load_now(pkg, &boot_pkg, 1);
    package_load(boot_pkg);
    package_flush(boot_pkg);

    stringid64_t boot_script = stringid64_from_string(cvar_get_string(_G.config.boot_script));
    luasys_execute_boot_script(boot_script);
}


static void _boot_unload() {
    stringid64_t boot_pkg = stringid64_from_string(cvar_get_string(_G.config.boot_pkg));
    stringid64_t pkg = stringid64_from_string("package");

    package_unload(boot_pkg);
    resource_unload(pkg, &boot_pkg, 1);

    //if (!cvar_get_int(_G.config.daemon)) {
        stringid64_t core_pkg = stringid64_from_string("core");
        resource_load_now(pkg, &core_pkg, 1);
        package_load(core_pkg);
        package_flush(core_pkg);
    //}

}

void application_start() {
#if defined(CETECH_DEVELOP)
    resource_set_autoload(1);
#else
    resource_set_autoload(0);
#endif

    if (cvar_get_int(_G.config.compile)) {
        resource_compiler_compile_all();

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

        renderer_create(_G.main_window);
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

    consolesrv_push_begin();
    while (_G.is_running) {
        struct scope_data application_sd = developsys_enter_scope("Application:update()");

        u64 now_ticks = cel_get_perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / cel_get_perf_freq();

        _G.dt = dt;
        last_tick = now_ticks;
        frame_time_accum += dt;


        machine_process();

        plugin_call_update();
        _G.game->update(dt);

        if(frame_time_accum >= frame_time) {
            if (!cvar_get_int(_G.config.daemon)) {
                struct scope_data render_sd = developsys_enter_scope("Game::render()");
                _G.game->render();
                developsys_leave_scope("Game::render()", render_sd);
            }

            frame_time_accum = 0.0f;
        }

        developsys_leave_scope("Application:update()", application_sd);
        developsys_push_record_float("engine.delta_time", dt);

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
