
#include <SDL2/SDL.h>
#include "celib/macros.h"
#include <celib/os/input.h>

static char *global_sdl_clipboard = NULL;

const char *get_clipboard_text() {
    if (global_sdl_clipboard) {
        SDL_free(global_sdl_clipboard);
    }

    global_sdl_clipboard = SDL_GetClipboardText();
    return global_sdl_clipboard;
}

void set_clipboard_text(const char *text) {
    SDL_SetClipboardText(text);
}

struct ce_os_input_a0 input_api = {
        .get_clipboard_text=get_clipboard_text,
        .set_clipboard_text=set_clipboard_text,
};

struct ce_os_input_a0 *ce_os_input_a0 = &input_api;