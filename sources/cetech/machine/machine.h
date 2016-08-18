#ifndef CETECH_MACHINE_H
#define CETECH_MACHINE_H

//==============================================================================
// Includes
//==============================================================================

#include <celib/math/types.h>
#include "cetech/machine/types.h"


typedef int (*machine_part_init_t)();
typedef void (*machine_part_shutdown_t)();
typedef void (*machine_part_process_t)(struct eventstream *stream);

//==============================================================================
// Interface
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

#endif //CETECH_MACHINE_H
