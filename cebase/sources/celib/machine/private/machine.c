//==============================================================================
// Includes
//==============================================================================

#include <celib/memory/memory.h>
#include "celib/containers/array.h"
#include "celib/machine/machine.h"
#include "celib/machine/private/sdl2/sdl_parts.h"
#include <engine/memory/memsys.h>

//==============================================================================
// Defines
//==============================================================================

#define MAX_PARTS 64
#define LOG_WHERE "machine"


//==============================================================================
// Globals
//==============================================================================

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

int machine_init(int stage) {
    if (stage == 0) {
        return 1;
    }


    _G = (struct G) {0};

    eventstream_create(&_G.eventstream, _memsys_main_allocator());

    machine_register_part("sdl", sdl_init, sdl_shutdown, sdl_process);
    machine_register_part("sdl_keyboard", sdl_keyboard_init, sdl_keyboard_shutdown, sdl_keyboard_process);
    machine_register_part("sdl_mouse", sdl_mouse_init, sdl_mouse_shutdown, sdl_mouse_process);
    machine_register_part("sdl_gamepad", sdl_gamepad_init, sdl_gamepad_shutdown, sdl_gamepad_process);

    for (int i = 0; i < _G.parts_count; ++i) {
        if (!_G.init[i]()) {
            log_error(LOG_WHERE, "Could not init machine part \"%s\"", _G.name[i]);

            for (i = i - 1; i >= 0; --i) {
                _G.shutdown[i]();
            }

            return 0;
        };
    }

    return 1;
}

void machine_shutdown() {
    eventstream_destroy(&_G.eventstream);

    for (int i = 0; i < _G.parts_count; ++i) {
        _G.shutdown[i]();
    }

    _G = (struct G) {0};
}

void machine_process() {
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

