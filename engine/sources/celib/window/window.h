#ifndef CETECH_WINDOW_H
#define CETECH_WINDOW_H

//==============================================================================
// Window interface
//==============================================================================

window_t llm_window_new(const char *title,
                        enum WindowPos x,
                        enum WindowPos y,
                        const i32 width,
                        const i32 height,
                        enum WindowFlags flags);

window_t llm_window_new_from(void *hndl);

void llm_window_destroy(window_t w);

void llm_window_set_title(window_t w,
                          const char *title);

const char *llm_window_get_title(window_t w);

void llm_window_update(window_t w);

void llm_window_resize(window_t w,
                       uint32_t width,
                       uint32_t height);

void *llm_window_native_window_ptr(window_t w);

void *llm_window_native_display_ptr(window_t w);


#endif //CETECH_WINDOW_H
