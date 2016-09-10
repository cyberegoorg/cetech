#ifndef CETECH_WINDOW_H
#define CETECH_WINDOW_H

#include "./types.h"
#include "../types.h"

//==============================================================================
// Window interface
//==============================================================================

window_t window_new(const char *title,
                    enum WindowPos x,
                    enum WindowPos y,
                    const i32 width,
                    const i32 height,
                    enum WindowFlags flags);

window_t window_new_from(void *hndl);

void window_destroy(window_t w);

void window_set_title(window_t w,
                      const char *title);

const char *window_get_title(window_t w);

void window_update(window_t w);

void window_resize(window_t w,
                   uint32_t width,
                   uint32_t height);

void window_get_size(window_t w,
                     uint32_t *width,
                     uint32_t *height);

void *window_native_window_ptr(window_t w);

void *window_native_display_ptr(window_t w);


#endif //CETECH_WINDOW_H
