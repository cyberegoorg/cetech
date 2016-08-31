//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>
#include <celib/os/time.h>
#include <celib/window/types.h>
#include <celib/window/window.h>
#include <celib/stringid/stringid.h>

#include "celib/log/log.h"
#include "celib/memory/memory.h"
#include "celib/containers/hash.h"

#include "engine/memory_system/memory_system.h"
#include "engine/input/input.h"
#include "engine/console_server/console_server.h"
#include "engine/task_manager/task_manager.h"

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

#define _G ApplicationGlobals

static struct G {
    window_t main_window;
    int is_running;
    int init_error;
    float dt;
} ApplicationGlobals = {0};

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

    log_debug(LOG_WHERE, "Init (global size: %lu)", sizeof(struct G));

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

    log_set_wid_clb(taskmanager_worker_id);

    return 1;
}

void application_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    if (!_G.init_error) {
        for (int i = _SYSTEMS_SIZE - 1; i >= 0; --i) {
            _SYSTEMS[i].shutdown();
        }

        window_destroy(_G.main_window);
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


static void _task1(void *d) {
    if (keyboard_button_state(keyboard_button_index("q"))) {
        application_quit();
    }

    if (mouse_button_state(mouse_button_index("left"))) {
        vec2f_t pos = {0};
        mouse_axis(mouse_axis_index("absolute"), pos);

        log_info("sdadsad", "time %f", _G.dt);

        if (pos[0] != 0.0f) {
            //log_info("sdadsad", "pos %f, %f", pos[0], pos[1]);
        }
    }

    //usleep(1 * 1000);
}

static void _input_task(void *d) {
    keyboard_process();
    mouse_process();
}

static void _consolesrv_task(void *d) {
    consolesrv_update();
}

void application_start() {
    resource_compiler_compile_all();

    stringid64_t lua_boot = stringid64_from_string("lua/boot");
    resource_load_now(stringid64_from_string("lua"), &lua_boot, 1);
    resource_get(stringid64_from_string("lua"), lua_boot);
    resource_reload(stringid64_from_string("lua"), &lua_boot, 1);
    resorucemanager_unload(stringid64_from_string("lua"), &lua_boot, 1);

    _G.main_window = window_new(
            "Cetech",
            WINDOWPOS_UNDEFINED,
            WINDOWPOS_UNDEFINED,
            800, 600,
            WINDOW_NOFLAG
    );

    _G.is_running = 1;
    uint32_t last_tick = os_get_ticks();
    while (_G.is_running) {
        uint32_t now_ticks = os_get_ticks();
        float dt = (now_ticks - last_tick) * 0.001f;
        _G.dt = dt;
        last_tick = now_ticks;

        machine_process();
        _dump_event();

        task_t frame_task = taskmanager_add_null("frame", task_null, task_null, TASK_PRIORITY_HIGH, TASK_AFFINITY_NONE);

        task_t consolesrv_task = taskmanager_add_begin(
                "consolesrv",
                _consolesrv_task,
                NULL,
                0,
                task_null,
                frame_task,
                TASK_PRIORITY_HIGH,
                TASK_AFFINITY_MAIN
        );

        task_t input_task = taskmanager_add_begin(
                "input",
                _input_task,
                NULL,
                0,
                task_null,
                frame_task,
                TASK_PRIORITY_HIGH,
                TASK_AFFINITY_MAIN
        );

        task_t task = taskmanager_add_begin(
                "task1",
                _task1,
                NULL,
                0,
                frame_task,
                task_null,
                TASK_PRIORITY_HIGH,
                TASK_AFFINITY_NONE
        );

        const task_t tasks[] = {task, input_task, consolesrv_task, frame_task};
        taskmanager_add_end(tasks, CE_ARRAY_LEN(tasks));

        taskmanager_wait(task);
        window_update(_G.main_window);
    }
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