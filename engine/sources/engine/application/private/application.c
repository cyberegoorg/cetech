//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>

#include <celib/os/time.h>
#include <celib/window/types.h>
#include <celib/window/window.h>
#include <celib/string/stringid.h>
#include <celib/os/process.h>
#include <celib/os/cmd_line.h>

#include <engine/application/application.h>
#include <celib/config/cvar.h>
#include <engine/machine/machine.h>
#include <engine/resource/resource.h>
#include <engine/luasys/luasys.h>
#include <engine/resource/resource.h>
#include <engine/renderer/renderer.h>
#include <engine/develop/develop_system.h>

#include "celib/memory/memory.h"
#include "engine/input/input.h"
#include "engine/develop/console_server.h"
#include "celib/task/task.h"

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

#define _G ApplicationGlobals

static struct G {
    struct {
        cvar_t cv_boot_pkg;
        cvar_t cv_boot_script;
        cvar_t cv_screen_x;
        cvar_t cv_screen_y;
        cvar_t cv_fullscreen;

        cvar_t cv_daemon;
        cvar_t cv_compile;
        cvar_t cv_continue;
        cvar_t cv_wait;
        cvar_t cv_wid;
    } config;

    const struct game_callbacks *game;
    window_t main_window;
    struct args args;
    int is_running;
    int init_error;
    float dt;
} ApplicationGlobals = {0};

//==============================================================================
// Systems
//==============================================================================

#include "engine/systems.h"

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

    _G.config.cv_boot_pkg = cvar_new_str("core.boot_pkg", "Boot package", "boot");
    _G.config.cv_boot_script = cvar_new_str("core.boot_script", "Boot script", "lua/boot");

    _G.config.cv_screen_x = cvar_new_int("screen.x", "Screen width", 1024);
    _G.config.cv_screen_y = cvar_new_int("screen.y", "Screen height", 768);
    _G.config.cv_fullscreen = cvar_new_int("screen.fullscreen", "Fullscreen", 0);

    _G.config.cv_daemon = cvar_new_int(".daemon", "Daemon mode", 0);
    _G.config.cv_compile = cvar_new_int(".compile", "Comple", 0);
    _G.config.cv_continue = cvar_new_int(".continue", "Continue after compile", 0);
    _G.config.cv_wait = cvar_new_int(".wait", "Wait for client", 0);
    _G.config.cv_wid = cvar_new_int(".wid", "Wid", 0);

    // Cvar stage
    for (int i = 0; i < STATIC_SYSTEMS_SIZE; ++i) {
        if (!_SYSTEMS[i].init(0)) {
            log_error(LOG_WHERE, "Could not init system \"%s\"", _SYSTEMS[i].name);
            _G.init_error = 1;
            return 0;
        }
    }

    cvar_parse_core_args(_G.args);

    if (cvar_get_int(_G.config.cv_compile)) {
        resource_compiler_create_build_dir();
        cvar_compile_global();
    }

    cvar_load_global();

    if (!cvar_parse_args(_G.args)) {
        return 0;
    }

    // main stage
    for (int i = 0; i < STATIC_SYSTEMS_SIZE; ++i) {
        if (!_SYSTEMS[i].init(1)) {
            log_error(LOG_WHERE, "Could not init system \"%s\"", _SYSTEMS[i].name);

            for (i = i - 1; i >= 0; --i) {
                _SYSTEMS[i].shutdown();
            }

            _G.init_error = 1;
            return 0;
        }
    }

    log_set_wid_clb(taskmanager_worker_id);

    consolesrv_register_command("wait", _cmd_wait);
    return 1;
}

int application_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    if (!_G.init_error) {
        for (int i = STATIC_SYSTEMS_SIZE - 1; i >= 0; --i) {
            _SYSTEMS[i].shutdown();
        }

        window_destroy(_G.main_window);
    }

    cvar_shutdown();
    memsys_shutdown();
    log_shutdown();

    return !_G.init_error;
}

static void _dump_event() {
    struct event_header *event = machine_event_begin();

    u32 size = 0;
    while (event != machine_event_end()) {
        size = size + 1;
        switch (event->type) {
//            case EVENT_GAMEPAD_DOWN:
//                log_info(LOG_WHERE, "Gamepad %d btn %d down", ((struct gamepad_btn_event*)event)->gamepad_id, ((struct gamepad_btn_event*)event)->button);
//                break;
//
//            case EVENT_GAMEPAD_UP:
//                log_info(LOG_WHERE, "Gamepad %d btn %d up", ((struct gamepad_btn_event*)event)->gamepad_id, ((struct gamepad_btn_event*)event)->button);
//                break;
//
//            case EVENT_GAMEPAD_MOVE:
//                log_info(LOG_WHERE, "Gamepad %d move axis %d [%f, %f]",
//                         ((struct gamepad_move_event*)event)->gamepad_id,
//                         ((struct gamepad_move_event*)event)->axis,
//                         ((struct gamepad_move_event*)event)->position.x,
//                         ((struct gamepad_move_event*)event)->position.y);
//                break;


//            case EVENT_KEYBOARD_DOWN:
//                log_info(LOG_WHERE, "Key down %d", ((struct keyboard_event*)event)->button);
//                break;

//            case EVENT_KEYBOARD_UP:
//                log_info(LOG_WHERE, "Key up %d", ((struct keyboard_event*)event)->button);
//                break;

//            case EVENT_MOUSE_UP:
//                log_info(LOG_WHERE, "mouse down %d", ((struct mouse_event *) event)->button);
//                break;

//            case EVENT_MOUSE_DOWN:
//                log_info(LOG_WHERE, "mouse up %d", ((struct mouse_event *) event)->button);
//                break;

//            case EVENT_MOUSE_MOVE:
//                log_info(LOG_WHERE, "mouse %f %f", ((struct mouse_move_event *) event)->pos[0],
//                         ((struct mouse_move_event *) event)->pos[1]);
//                break;

            default:
                break;
        }
        event = machine_event_next(event);
    }
}

static void _boot_stage() {
    stringid64_t boot_pkg = stringid64_from_string(cvar_get_string(_G.config.cv_boot_pkg));
    stringid64_t pkg = stringid64_from_string("package");

    // TODO: remove, this must be done by boot_package and load in boot_script
    if (!cvar_get_int(_G.config.cv_daemon)) {
        stringid64_t core_pkg = stringid64_from_string("core");
        resource_load_now(pkg, &core_pkg, 1);
        package_load(core_pkg);
        package_flush(core_pkg);
    }

    resource_load_now(pkg, &boot_pkg, 1);
    package_load(boot_pkg);
    package_flush(boot_pkg);

    stringid64_t boot_script = stringid64_from_string(cvar_get_string(_G.config.cv_boot_script));
    luasys_execute_boot_script(boot_script);
}


static void _boot_unload() {
    stringid64_t boot_pkg = stringid64_from_string(cvar_get_string(_G.config.cv_boot_pkg));
    stringid64_t pkg = stringid64_from_string("package");

    package_unload(boot_pkg);
    resource_unload(pkg, &boot_pkg, 1);

    if (!cvar_get_int(_G.config.cv_daemon)) {
        stringid64_t core_pkg = stringid64_from_string("core");
        resource_load_now(pkg, &core_pkg, 1);
        package_load(core_pkg);
        package_flush(core_pkg);
    }

}

void application_start() {
#if defined(CETECH_DEVELOP)
    resource_set_autoload(1);
#else
    resource_set_autoload(0);
#endif

    if (cvar_get_int(_G.config.cv_compile)) {
        resource_compiler_compile_all();

        if (!cvar_get_int(_G.config.cv_continue)) {
            return;
        }
    }

    if (!cvar_get_int(_G.config.cv_daemon)) {
        intptr_t wid = cvar_get_int(_G.config.cv_wid);

        char title[128] = {0};
        snprintf(title, CE_ARRAY_LEN(title), "cetech - %s", cvar_get_string(_G.config.cv_boot_script));

        if (wid == 0) {
            _G.main_window = window_new(
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    cvar_get_int(_G.config.cv_screen_x), cvar_get_int(_G.config.cv_screen_y),
                    cvar_get_int(_G.config.cv_fullscreen) ? WINDOW_FULLSCREEN : WINDOW_NOFLAG
            );
        } else {
            _G.main_window = window_new_from((void *) wid);
        }

        renderer_create(_G.main_window);
    }

    _boot_stage();

    u64 last_tick = os_get_perf_counter();
    _G.game = luasys_get_game_callbacks();

    if (!_G.game->init()) {
        log_error(LOG_WHERE, "Could not init game.");
        return;
    };

    _G.is_running = 1;
    log_info("core.ready", "Run main loop");

    float lag = 0.0f;
    float frame_time = 1.0f/ 1000.0f;//1.0f / frame_limit;

    consolesrv_push_begin();
    while (_G.is_running) {
        struct scope_data application_sd = developsys_enter_scope("Application:update()");

        u64 now_ticks = os_get_perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / os_get_perf_freq();

        _G.dt = dt;
        last_tick = now_ticks;

        lag += dt;


        while(lag >= frame_time) {
            machine_process();
            _dump_event();
            keyboard_process();
            mouse_process();
            gamepad_process();

            consolesrv_update();

            _G.game->update(frame_time);
            lag -= frame_time;
        }

        if (!cvar_get_int(_G.config.cv_daemon)) {
            struct scope_data render_sd = developsys_enter_scope("Game::render()");
            _G.game->render();
            window_update(_G.main_window);
            developsys_leave_scope("Game::render()", render_sd);
        }

        developsys_push_record_float("engine.delta_time", dt);
        developsys_leave_scope("Application:update()", application_sd);
        developsys_update();
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
