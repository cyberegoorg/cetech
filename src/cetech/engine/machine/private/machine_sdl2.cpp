//==============================================================================
// Includes
//==============================================================================

#include "celib/eventstream.inl"

#include "cetech/core/memory/memory.h"
#include "cetech/core/module/module.h"
#include "cetech/core/config/config.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/log/log.h"

#include "cetech/engine/resource/resource.h"
#include "cetech/engine/machine/machine.h"

#include <cetech/engine/application/application.h>

#include <include/SDL2/SDL.h>

CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_app_a0);

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


//==============================================================================
// Mouse part
//==============================================================================

extern int sdl_mouse_init(struct ct_api_a0 *api);

extern void sdl_mouse_shutdown();

extern void sdl_mouse_process(EventStream &stream);

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
namespace machine_sdl {
    ct_event_header *machine_event_begin() {
        return eventstream::begin<ct_event_header>(_G.eventstream);
    }


    ct_event_header *machine_event_end() {
        return eventstream::end<ct_event_header>(_G.eventstream);
    }

    ct_event_header *machine_event_next(ct_event_header *header) {
        return eventstream::next(header);
    }

    void _update(float dt) {
        eventstream::clear(_G.eventstream);
        SDL_Event e;

        while (SDL_PollEvent(&e) > 0) {
            switch (e.type) {
                case SDL_QUIT: {
                    ct_app_a0.quit();
                }
                    break;

                case SDL_CONTROLLERDEVICEADDED:
                case SDL_CONTROLLERDEVICEREMOVED:
                    sdl_gamepad_process_event(&e, _G.eventstream);
                    break;
            }
        }

        sdl_gamepad_process(_G.eventstream);
        sdl_mouse_process(_G.eventstream);
        sdl_keyboard_process(_G.eventstream);
    }

    static ct_machine_a0 a0 = {
            .event_begin = machine_sdl::machine_event_begin,
            .event_end = machine_sdl::machine_event_end,
            .event_next = machine_sdl::machine_event_next,
            .gamepad_is_active = sdl_gamepad_is_active,
            .gamepad_play_rumble = sdl_gamepad_play_rumble,
    };

    void init(struct ct_api_a0 *api) {
        api->register_api("ct_machine_a0", &a0);

        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_log_a0);
        CETECH_GET_API(api, ct_app_a0);

        ct_app_a0.register_on_update(machine_sdl::_update);

        _G.eventstream.init(ct_memory_a0.main_allocator());

        if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO) != 0) {
            ct_log_a0.error(LOG_WHERE, "Could not init sdl - %s",
                            SDL_GetError());
            return; // TODO: dksandasdnask FUCK init without return type?????
        }

        sdl_window_init(api);
        sdl_gamepad_init(api);
        sdl_mouse_init(api);
        sdl_keyboard_init(api);

    }

    void shutdown() {
        sdl_gamepad_shutdown();
        sdl_mouse_shutdown();
        sdl_keyboard_shutdown();
        sdl_window_shutdown();

        _G.eventstream.destroy();

        SDL_Quit();
    }
}


CETECH_MODULE_DEF(
        machine,
        {
            CETECH_GET_API(api, ct_log_a0);

            ct_api_a0 = *api;
        },
        {
            machine_sdl::init(api);
        },
        {
            CEL_UNUSED(api);

            CEL_UNUSED(api);
            machine_sdl::shutdown();

        }
)
