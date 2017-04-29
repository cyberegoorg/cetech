//==============================================================================
// Includes
//==============================================================================

#include "cetech/containers/array.inl"
#include <cetech/machine/machine.h>
#include "sdl2/sdl_parts.h"
#include <cetech/memory/memory.h>

//==============================================================================
// Defines
//==============================================================================

#define MAX_PARTS 64
#define LOG_WHERE "machine"


//==============================================================================
// Globals
//==============================================================================

typedef int (*machine_part_init_t)(get_api_fce_t);

typedef void (*machine_part_shutdown_t)();

typedef void (*machine_part_process_t)(struct eventstream *stream);


static struct G {
    machine_part_init_t init[MAX_PARTS];
    machine_part_shutdown_t shutdown[MAX_PARTS];
    machine_part_process_t process[MAX_PARTS];
    const char *name[MAX_PARTS];

    struct eventstream eventstream;
    int parts_count;
} _G = {0};


//==============================================================================
// Interface
//==============================================================================

void machine_register_part(const char *name,
                           machine_part_init_t init,
                           machine_part_shutdown_t shutdown,
                           machine_part_process_t process) {

    const int idx = _G.parts_count++;

    _G.name[idx] = name;
    _G.init[idx] = init;
    _G.shutdown[idx] = shutdown;
    _G.process[idx] = process;
}

static void _init(get_api_fce_t get_engine_api) {
    _G = (struct G) {0};

    eventstream_create(&_G.eventstream, _memsys_main_allocator());

    machine_register_part("sdl", sdl_init, sdl_shutdown, sdl_process);
    machine_register_part("sdl_keyboard", sdl_keyboard_init,
                          sdl_keyboard_shutdown, sdl_keyboard_process);
    machine_register_part("sdl_mouse", sdl_mouse_init, sdl_mouse_shutdown,
                          sdl_mouse_process);
    machine_register_part("sdl_gamepad", sdl_gamepad_init, sdl_gamepad_shutdown,
                          sdl_gamepad_process);

    for (int i = 0; i < _G.parts_count; ++i) {
        if (!_G.init[i](get_engine_api)) {
            log_error(LOG_WHERE, "Could not init machine part \"%s\"",
                      _G.name[i]);

            for (i = i - 1; i >= 0; --i) {
                _G.shutdown[i]();
            }

            return; //return 0;
        };
    }

    //return 1;
}

static void _shutdown() {
    eventstream_destroy(&_G.eventstream);

    for (int i = 0; i < _G.parts_count; ++i) {
        _G.shutdown[i]();
    }

    _G = (struct G) {0};
}

void _update() {
    eventstream_clear(&_G.eventstream);

    for (int i = 0; i < _G.parts_count; ++i) {
        _G.process[i](&_G.eventstream);
    }
}

struct event_header *machine_event_begin() {
    return eventstream_begin(&_G.eventstream);
}


struct event_header *machine_event_end() {
    return eventstream_end(&_G.eventstream);
}

struct event_header *machine_event_next(struct event_header *header) {
    return eventstream_next(header);
}

int machine_gamepad_is_active(int idx);

void machine_gamepad_play_rumble(int gamepad,
                                 float strength,
                                 uint32_t length);

void *machine_get_module_api(int api) {

    if (api == PLUGIN_EXPORT_API_ID) {
        static struct module_api_v0 module = {0};

        module.init = _init;
        module.shutdown = _shutdown;
        module.update = _update;

        return &module;

    } else if (api == MACHINE_API_ID) {
        static struct machine_api_v0 api_v1 = {
                .event_begin = machine_event_begin,
                .event_end = machine_event_end,
                .event_next = machine_event_next,
                .gamepad_is_active = machine_gamepad_is_active,
                .gamepad_play_rumble = machine_gamepad_play_rumble,
        };

        return &api_v1;
    }

    return 0;
}
