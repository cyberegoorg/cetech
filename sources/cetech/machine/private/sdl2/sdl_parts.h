#ifndef CETECH_SDL_PARTS_H
#define CETECH_SDL_PARTS_H

//==============================================================================
// Main part
//==============================================================================

int sdl_init();
void sdl_shutdown();
void sdl_process(struct eventstream *stream);


//==============================================================================
// Keyboard part
//==============================================================================

int sdl_keyboard_init();
void sdl_keyboard_shutdown();
void sdl_keyboard_process(struct eventstream *stream);


//==============================================================================
// Mouse part
//==============================================================================

int sdl_mouse_init();
void sdl_mouse_shutdown();
void sdl_mouse_process(struct eventstream *stream);

#endif //CETECH_SDL_PARTS_H
