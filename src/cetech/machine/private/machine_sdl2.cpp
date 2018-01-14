//==============================================================================
// Includes
//==============================================================================

#include "celib/eventstream.inl"

#include "cetech/os/memory.h"
#include "cetech/module/module.h"
#include "cetech/config/config.h"
#include "cetech/api/api_system.h"
#include "cetech/log/log.h"

#include "cetech/resource/resource.h"
#include "cetech/machine/machine.h"

#include <cetech/application/application.h>

#include <include/SDL2/SDL.h>

CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_api_a0);

using namespace celib;

//==============================================================================
// Extern functions
//==============================================================================

//==============================================================================
// Keyboard part
//==============================================================================

extern int sdl_keyboard_init(struct ct_api_a0 *api);

extern void sdl_keyboard_shutdown();

extern void sdl_keyboard_process(EventStream &stream);

extern void sdl_keyboard_process(SDL_Event *event,
                                 EventStream &stream);


//==============================================================================
// Mouse part
//==============================================================================

extern int sdl_mouse_init(struct ct_api_a0 *api);

extern void sdl_mouse_shutdown();

extern void sdl_mouse_process(EventStream &stream);

extern void sdl_mouse_process(SDL_Event *event,
                              EventStream &stream);

//==============================================================================
// Gamepad part
//==============================================================================

extern int sdl_gamepad_init(struct ct_api_a0 *api);

extern void sdl_gamepad_shutdown();

extern void sdl_gamepad_process(EventStream &stream);

extern void sdl_gamepad_process_event(SDL_Event *event,
                                      EventStream &stream);

int sdl_gamepad_is_active(int idx);

void sdl_gamepad_play_rumble(int gamepad,
                             float strength,
                             uint32_t length);

extern int sdl_window_init(struct ct_api_a0 *api);

extern void sdl_window_shutdown();

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "machine"


//==============================================================================
// Globals
//==============================================================================

static struct MachineGlobals {
    EventStream eventstream;
} _G;

CETECH_DECL_API(ct_memory_a0);

//==============================================================================
// Interface
//==============================================================================
static ct_event_header *machine_event_begin() {
    return eventstream::begin<ct_event_header>(_G.eventstream);
}


static ct_event_header *machine_event_end() {
    return eventstream::end<ct_event_header>(_G.eventstream);
}

static ct_event_header *machine_event_next(ct_event_header *header) {
    return eventstream::next(header);
}

static void _update(float dt) {
    CEL_UNUSED(dt);

    eventstream::clear(_G.eventstream);
    SDL_Event e;

    sdl_gamepad_process(_G.eventstream);
    sdl_mouse_process(_G.eventstream);
    sdl_keyboard_process(_G.eventstream);

    while (SDL_PollEvent(&e) > 0) {
        switch (e.type) {
            case SDL_QUIT: {
                ct_event_header ev;
                eventstream::push<ct_event_header>(_G.eventstream,
                                                   EVENT_QUIT,
                                                   ev);
            }
                break;

            case SDL_WINDOWEVENT: {
                switch (e.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        ct_window_resized_event ev;
                        ev.window_id = e.window.windowID;
                        ev.width = e.window.data1;
                        ev.height = e.window.data2;

                        eventstream::push<ct_event_header>(_G.eventstream,
                                                           EVENT_WINDOW_RESIZED,
                                                           ev);
                    }
                        break;
                }
            }
                break;


            default:
                sdl_mouse_process(&e, _G.eventstream);
                sdl_gamepad_process_event(&e, _G.eventstream);
                sdl_keyboard_process(&e, _G.eventstream);
                break;
        }
    }

}

static ct_machine_a0 a0 = {
        .update = _update,
        .event_begin = machine_event_begin,
        .event_end = machine_event_end,
        .event_next = machine_event_next,
        .gamepad_is_active = sdl_gamepad_is_active,
        .gamepad_play_rumble = sdl_gamepad_play_rumble,
};

static void init(struct ct_api_a0 *api) {
    api->register_api("ct_machine_a0", &a0);

    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_log_a0);

    _G.eventstream.init(ct_memory_a0.main_allocator());

    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO) != 0) {
        //if (SDL_Init(0) != 0) {
        ct_log_a0.error(LOG_WHERE, "Could not init sdl - %s",
                        SDL_GetError());
        return; // TODO: dksandasdnask FUCK init without return type?????
    }

    sdl_window_init(api);
    sdl_gamepad_init(api);
    sdl_mouse_init(api);
    sdl_keyboard_init(api);

}

static void shutdown() {
    sdl_gamepad_shutdown();
    sdl_mouse_shutdown();
    sdl_keyboard_shutdown();
    sdl_window_shutdown();

    _G.eventstream.destroy();

    SDL_Quit();
}

CETECH_MODULE_DEF(
        machine,
        {
            CETECH_GET_API(api, ct_log_a0);

            ct_api_a0 = *api;
        },
        {
            CEL_UNUSED(reload);
            init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            shutdown();

        }
)
