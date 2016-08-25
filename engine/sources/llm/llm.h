#ifndef CETECH_MACHINE_H
#define CETECH_MACHINE_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/math/types.h"
#include "types.h"
#include "thread_types.h"

typedef int (*machine_part_init_t)();

typedef void (*machine_part_shutdown_t)();

typedef void (*machine_part_process_t)(struct eventstream *stream);

//==============================================================================
// Machine interface
//==============================================================================

int llm_init();

void llm_shutdown();

void llm_register_part(const char *name,
                       machine_part_init_t init,
                       machine_part_shutdown_t shutdown,
                       machine_part_process_t process);

void llm_process();

struct event_header *llm_event_begin();

struct event_header *llm_event_end();

struct event_header *llm_event_next(struct event_header *header);

//==============================================================================
// CPU
// ==============================================================================

int llm_cpu_count();

//==============================================================================
// Time
// ==============================================================================

u32 llm_get_ticks();

//==============================================================================
// SO
//==============================================================================

void *llm_load_object(const char *path);

void llm_unload_object(void *so);

void *llm_load_function(void *so, void *name);

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

#endif //CETECH_MACHINE_H
