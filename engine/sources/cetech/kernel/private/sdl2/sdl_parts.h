#ifndef CETECH_SDL_PARTS_H
#define CETECH_SDL_PARTS_H

//==============================================================================
// Main part
//==============================================================================

#include <SDL2/SDL_events.h>

int sdl_init( struct api_v0 *api);

void sdl_shutdown();

void sdl_process(struct eventstream *stream);


//==============================================================================
// Keyboard part
//==============================================================================

int sdl_keyboard_init( struct api_v0 *api);

void sdl_keyboard_shutdown();

void sdl_keyboard_process(struct eventstream *stream);


//==============================================================================
// Mouse part
//==============================================================================

int sdl_mouse_init( struct api_v0* api);

void sdl_mouse_shutdown();

void sdl_mouse_process(struct eventstream *stream);

//==============================================================================
// Gamepad part
//==============================================================================

int sdl_gamepad_init( struct api_v0 *api);

void sdl_gamepad_shutdown();

void sdl_gamepad_process(struct eventstream *stream);

void sdl_gamepad_process_event(SDL_Event *event,
                               struct eventstream *stream);

#endif //CETECH_SDL_PARTS_H
