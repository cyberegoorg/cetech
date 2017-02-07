#ifndef CETECH_DEVELOP_SYSTEM_H
#define CETECH_DEVELOP_SYSTEM_H

#include "celib/types.h"
#include "celib/containers/eventstream.h"
#include "types.h"


//==============================================================================
// Interface
//==============================================================================


void developsys_update(float dt);


void _developsys_push(struct event_header *header,
                      u32 type,
                      u64 size);

void developsys_push_record_float(const char *name,
                                  float value);

void developsys_push_record_int(const char *name,
                                int value);

struct scope_data developsys_enter_scope(const char *name);

void developsys_leave_scope(const char *name,
                            struct scope_data scope_data);


#endif //CETECH_DEVELOP_SYSTEM_H
