#ifndef CETECH_MACHINE_H
#define CETECH_MACHINE_H

//==============================================================================
// Includes
//==============================================================================

#include <celib/math/types.h>
#include "cetech/machine/types.h"


//==============================================================================
// Interface
//==============================================================================

int machine_init();

void machine_shutdown();

void machine_begin_frame();

void machine_end_frame();

struct event_header *machine_event_begin();

struct event_header *machine_event_end();

struct event_header *machine_event_next(struct event_header *header);

#endif //CETECH_MACHINE_H
