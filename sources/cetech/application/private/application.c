//==============================================================================
// Includes
//==============================================================================

#include <celib/log/log.h>
#include <celib/memory/memory.h>

#include "../../machine/machine.h"
#include "../../input/input.h"
#include "../../consoleserver/consoleserver.h"

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

static struct G {
    window_t main_window;
    int is_running;
    int init_error;
} _G = {0};

//==============================================================================
// Systems
//==============================================================================

#include "systems.h"

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

int application_init(int argc, char **argv) {
    _G = (struct G) {0};

    log_init(_get_worker_id);
    log_register_handler(log_stdout_handler, NULL);

    log_debug(LOG_WHERE, "Init");

    memsys_init(4 * 1024 * 1024);

    for (int i = 0; i < _SYSTEMS_SIZE; ++i) {
        if (!_SYSTEMS[i].init()) {
            log_error(LOG_WHERE, "Could not init system \"%s\"", _SYSTEMS[i].name);

            for (i = i - 1; i >= 0; --i) {
                _SYSTEMS[i].shutdown();
            }

            _G.init_error = 1;
            return 0;
        }
    }

    return 1;
}

void application_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    if (!_G.init_error) {
        for (int i = _SYSTEMS_SIZE - 1; i >= 0; --i) {
            _SYSTEMS[i].shutdown();
        }

        machine_window_destroy(_G.main_window);
    }

    memsys_shutdown();
    log_shutdown();
}

static void _dump_event() {
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
    _G.main_window = machine_window_new(
            "Cetech",
            WINDOWPOS_UNDEFINED,
            WINDOWPOS_UNDEFINED,
            800, 600,
            WINDOW_NOFLAG
    );

    _G.is_running = 1;
    while (_G.is_running) {
        machine_process();
        _dump_event();

        keyboard_process();
        mouse_process();

        consolesrv_update();
        machine_window_update(_G.main_window);

        if (keyboard_button_state(keyboard_button_index("q"))) {
            application_quit();
        }

        if (mouse_button_state(mouse_button_index("left"))) {
            vec2f_t pos = {0};
            mouse_axis(mouse_axis_index("absolute"), pos);

            if (pos[0] != 0.0f) {
                log_info("sdadsad", "pos %f, %f", pos[0], pos[1]);
            }
        }
    }
}

