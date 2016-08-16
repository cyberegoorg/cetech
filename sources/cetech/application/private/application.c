//==============================================================================
// Includes
//==============================================================================

#include <celib/memory/memory.h>
#include <cetech/luasystem/luasystem.h>
#include <cetech/machine/machine.h>
#include <cetech/input/input.h>
#include <celib/log/log.h>
#include <cetech/configsystem/configsystem.h>
#include <cetech/consoleserver/consoleserver.h>
#include "cetech/windowsystem/windowsystem.h"

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

static struct G {
    int is_running;
    window_t main_window;
} _G = {0};


//==============================================================================
// Private
//==============================================================================

static char _get_worker_id() {
    return 0;
}

//==============================================================================
// LUA inteface
//==============================================================================
#define LUA_MODULE_NAME "App"

static int _application_quit(lua_State *l) {
    void application_quit();

    application_quit();
    return 0;
}

static void _register_lua_fce() {
    LUA.add_module_function(LUA_MODULE_NAME, "quit", _application_quit);
}

//==============================================================================
// Interface
//==============================================================================

void application_quit() {
    _G.is_running = 0;
}

int application_init(int argc, char **argv) {
    _G = (struct G) {0};

    log_init(_get_worker_id);
    log_register_handler(log_stdout_handler, NULL);

    memsys_init(4 * 1024 * 1024);
    config_init();

    consolesrv_init();

    LUA.init();

    if (!machine_init()) {
        return 0;
    };

    if (!keyboard_init()) {
        return 0;
    };

    if (!mouse_init()) {
        return 0;
    };

    windowsys_init();
    _register_lua_fce();

    return 1;
}

void application_shutdown() {
    windowsys_destroy_window(_G.main_window);

    windowsys_shutdown();
    keyboard_shutdow();
    mouse_shutdow();
    machine_shutdown();

    LUA.shutdown();
    consolesrv_shutdown();
    config_shutdown();
    memsys_shutdown();
    log_shutdown();
}

void dump_event() {
    struct event_header *event = machine_event_begin();

    u32 size = 0;
    while (event != machine_event_end()) {
        size = size + 1;
        switch (event->type) {
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

void application_start() {
    _G.main_window = windowsys_new_window(
            "Cetech",
            WINDOWPOS_UNDEFINED,
            WINDOWPOS_UNDEFINED,
            800, 600,
            WINDOW_NOFLAG
    );

    _G.is_running = 1;
    while (_G.is_running) {
        machine_begin_frame();
        dump_event();

        keyboard_process();
        mouse_process();

        consolesrv_update();
        windowsys_update(_G.main_window);

        if (keyboard_button_state(keyboard_button_index("q"))) {
            application_quit();
        }

        if (mouse_button_state(mouse_button_index("left"))) {
            vec2f_t pos = {0};
            mouse_axis(mouse_axis_index("delta"), pos);

            if (pos[0] != 0.0f) {
                log_info("sdadsad", "pos %f, %f", pos[0], pos[1]);
            }
        }

        machine_end_frame();
    }
}

