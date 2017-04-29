#ifndef CELIB_SDL_PARTS_H
#define CELIB_SDL_PARTS_H

//==============================================================================
// Main part
//==============================================================================

#include <SDL2/SDL_events.h>

int sdl_init(get_api_fce_t get_engine_api);

void sdl_shutdown();

void sdl_process(struct eventstream *stream);


//==============================================================================
// Keyboard part
//==============================================================================

int sdl_keyboard_init(get_api_fce_t get_engine_api);

void sdl_keyboard_shutdown();

void sdl_keyboard_process(struct eventstream *stream);


//==============================================================================
// Mouse part
//==============================================================================

int sdl_mouse_init(get_api_fce_t get_engine_api);

void sdl_mouse_shutdown();

void sdl_mouse_process(struct eventstream *stream);

//==============================================================================
// Gamepad part
//==============================================================================

int sdl_gamepad_init(get_api_fce_t get_engine_api);

void sdl_gamepad_shutdown();

void sdl_gamepad_process(struct eventstream *stream);

void sdl_gamepad_process_event(SDL_Event *event,
                               struct eventstream *stream);

#endif //CELIB_SDL_PARTS_H
