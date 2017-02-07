#ifndef CETECH_DEVELOP_TYPES_H
#define CETECH_DEVELOP_TYPES_H

#include "include/mpack/mpack.h"

#include "celib/types.h"
#include "celib/math/types.h"
#include "celib/containers/eventstream.h"

//==============================================================================
// Conosle Server
//==============================================================================

typedef int (*console_server_command_t)(mpack_node_t,
                                        mpack_writer_t *);

struct ConsoleServerApiV1 {
    void (*consolesrv_push_begin)();

    void (*consolesrv_register_command)(const char *,
                                        console_server_command_t);
};

//==============================================================================
// Develop system
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

struct scope_data {
    time_t start;
    u64 start_timer;
};

#define developsys_push(type, event) _developsys_push((struct event_header*)(&event), type, sizeof(event))


#endif //CETECH_DEVELOP_TYPES_H
