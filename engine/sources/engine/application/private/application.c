//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>
#include <celib/os/time.h>
#include <celib/window/types.h>
#include <celib/window/window.h>
#include <celib/stringid/stringid.h>
#include <engine/application/application.h>
#include <engine/cvar/cvar.h>
#include <celib/os/process.h>
#include <engine/machine/machine.h>
#include <engine/resource_manager/resource_manager.h>
#include <engine/lua_system/lua_system.h>
#include <engine/resource_compiler/resource_compiler.h>
#include <engine/renderer/renderer.h>
#include <engine/entcom/types.h>
#include <engine/world_system/unit_system.h>
#include <engine/world_system/transform.h>
#include <engine/world_system/world_system.h>
#include <engine/renderer/material.h>
#include <celib/math/types.h>

#include "celib/containers/map.h"

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
    const struct game_callbacks *game;
    window_t main_window;
    cvar_t cv_boot_pkg;
    cvar_t cv_boot_script;
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

int application_init(int argc,
                     char **argv) {
    _G = (struct G) {0};

    log_init(_get_worker_id);
    log_register_handler(log_stdout_handler, NULL);

    logdb_init_db(".");

    log_debug(LOG_WHERE, "Init (global size: %lu)", sizeof(struct G));

    memsys_init(4 * 1024 * 1024);

    _G.cv_boot_pkg = cvar_new_str("application.boot_pkg", "Boot package", "boot");
    _G.cv_boot_script = cvar_new_str("application.boot_scrpt", "Boot script", "lua/boot");

    for (int i = 0; i < STATIC_SYSTEMS_SIZE; ++i) {
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

int application_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    if (!_G.init_error) {
        for (int i = STATIC_SYSTEMS_SIZE - 1; i >= 0; --i) {
            _SYSTEMS[i].shutdown();
        }

        window_destroy(_G.main_window);
    }

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


static void _input_task(void *d) {
    keyboard_process();
    mouse_process();
    gamepad_process();
}

static void _consolesrv_task(void *d) {
    consolesrv_update();
}

static void _game_update_task(void *d) {
    float *dt = d;

    _G.game->update(*dt);
}

static void _game_render_task(void *d) {
    _G.game->render();
}

static void _boot_stage() {
    stringid64_t boot_pkg = stringid64_from_string(cvar_get_string(_G.cv_boot_pkg));
    stringid64_t pkg = stringid64_from_string("package");

    resource_load_now(pkg, &boot_pkg, 1);
    package_load(boot_pkg);
    package_flush(boot_pkg);

    stringid64_t boot_script = stringid64_from_string(cvar_get_string(_G.cv_boot_script));
    luasys_execute_boot_script(boot_script);
}


static void _boot_unload() {
    stringid64_t boot_pkg = stringid64_from_string(cvar_get_string(_G.cv_boot_pkg));
    stringid64_t pkg = stringid64_from_string("package");

    package_unload(boot_pkg);
    resource_unload(pkg, &boot_pkg, 1);
}

void application_start() {
    resource_set_autoload(1);
    resource_compiler_compile_all();


    _G.main_window = window_new(
            "Cetech",
            WINDOWPOS_UNDEFINED,
            WINDOWPOS_UNDEFINED,
            1024, 768,
            WINDOW_NOFLAG
    );
    renderer_create(_G.main_window);

    _boot_stage();

//////////////////
    void *data = resource_get(stringid64_from_string("unit"), stringid64_from_string("unit1"));
    world_t w = world_create();
    entity_t e = unit_spawn_from_resource(w, data);
    transform_t t = transform_get(w, e);
    vec3f_t s = transform_get_scale(w, t);

//    material_set_texture(m, res, stringid64_from_string("dasdasdsadasdasdas"));
//
////////////

    uint32_t last_tick = os_get_ticks();
    _G.game = luasys_get_game_callbacks();

    if (!_G.game->init()) {
        log_error(LOG_WHERE, "Could not init game.");
        return;
    };

    _G.is_running = 1;
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

        task_t game_update = taskmanager_add_begin(
                "game_update",
                _game_update_task,
                &dt,
                sizeof(float *),
                input_task,
                frame_task,
                TASK_PRIORITY_HIGH,
                TASK_AFFINITY_MAIN
        );

        task_t game_render = taskmanager_add_begin(
                "game_render",
                _game_render_task,
                NULL,
                0,
                game_update,
                frame_task,
                TASK_PRIORITY_HIGH,
                TASK_AFFINITY_MAIN
        );

        const task_t tasks[] = {
                input_task,
                consolesrv_task,
                game_render,
                game_update,
                frame_task
        };

        taskmanager_add_end(tasks, CE_ARRAY_LEN(tasks));

        taskmanager_wait(frame_task);


        window_update(_G.main_window);
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
