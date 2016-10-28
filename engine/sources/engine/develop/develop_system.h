#ifndef CETECH_DEVELOP_SYSTEM_H
#define CETECH_DEVELOP_SYSTEM_H

#include "celib/types.h"
#include "celib/containers/eventstream.h"

//==============================================================================
// Typedefs
//==============================================================================


enum {
    EVENT_NULL = 0,
    EVENT_SCOPE,
    EVENT_RECORD_FLOAT,
    EVENT_RECORD_INT,
};

struct record_float_event {
    struct event_header header;
    char name[64];
    float value;
};

struct record_int_event {
    struct event_header header;
    char name[64];
    i32 value;
};

struct scope_event {
    struct event_header header;
    char name[64];
    time_t start;
    float duration;
    u32 depth;
    u32 worker_id;
};

//==============================================================================
// Interface
//==============================================================================


int developsys_init(int stage);

void developsys_shutdown();

void developsys_update();

#define developsys_push(type, event) _developsys_push((struct event_header*)(&event), type, sizeof(event))

void _developsys_push(struct event_header *header,
                      u32 type,
                      u64 size);

void developsys_push_record_float(const char *name,
                                  float value);

void developsys_push_record_int(const char *name,
                                int value);

struct scope_data {
    time_t start;
    u64 start_timer;
};

struct scope_data developsys_enter_scope(const char *name);

void developsys_leave_scope(const char *name,
                            struct scope_data scope_data);


#endif //CETECH_DEVELOP_SYSTEM_H
