#ifndef CETECH_MACHINE_H
#define CETECH_MACHINE_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/math/types.h"
#include "types.h"

typedef int (*machine_part_init_t)();
typedef void (*machine_part_shutdown_t)();
typedef void (*machine_part_process_t)(struct eventstream *stream);

//==============================================================================
// Machine interface
//==============================================================================

int machine_init();

void machine_shutdown();

void machine_register_part(const char* name,
                           machine_part_init_t init,
                           machine_part_shutdown_t shutdown,
                           machine_part_process_t process);

void machine_process();

struct event_header *machine_event_begin();

struct event_header *machine_event_end();

struct event_header *machine_event_next(struct event_header *header);

//==============================================================================
// Window interface
//==============================================================================

window_t machine_window_new(const char *title,
                            enum WindowPos x,
                            enum WindowPos y,
                            const i32 width,
                            const i32 height,
                            enum WindowFlags flags);

window_t machine_window_new_from(void *hndl);

void machine_window_destroy(window_t w);

void machine_window_set_title(window_t w,
                              const char *title);

const char *machine_window_get_title(window_t w);

void machine_window_update(window_t w);

void machine_window_resize(window_t w,
                           uint32_t width,
                           uint32_t height);

void* machine_window_native_window_ptr(window_t w);
void* machine_window_native_display_ptr(window_t w);

#endif //CETECH_MACHINE_H
