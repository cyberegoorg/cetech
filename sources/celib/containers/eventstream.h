#ifndef CETECH_EVENTSTREAM_H
#define CETECH_EVENTSTREAM_H

//==============================================================================
// Includes
//==============================================================================

#include "array.h"
#include "types.h"
#include "../types.h"

//==============================================================================
// Interface
//==============================================================================

static inline void eventstream_create(struct eventstream *es, struct allocator *allocator) {
    ARRAY_INIT(u8, &es->stream, allocator);
}

static inline void eventstream_destroy(struct eventstream *es) {
    ARRAY_DESTROY(u8, &es->stream);
}

static inline void eventstream_clear(struct eventstream *es) {
    ARRAY_RESIZE(u8, &es->stream, 0);
}

static inline struct event_header *eventstream_begin(struct eventstream *es) {
    return (struct event_header *) ARRAY_BEGIN(&es->stream);
}

static inline struct event_header *eventstream_end(struct eventstream *es) {
    return (struct event_header *) ARRAY_END(&es->stream);
}

static inline struct event_header *eventstream_next(struct event_header *header) {
    return (struct event_header *) (((char *) header) + header->size);
}

#define event_stream_push(es, type, event) _eventstream_push(es, (struct event_header*)(&event), type, sizeof(event))
static inline void _eventstream_push(struct eventstream *es, struct event_header *header, u32 type, u64 size) {
    header->type = type;
    header->size = size;
    array_push_u8(&es->stream, (u8 *) header, size);
}

#endif //CETECH_EVENTSTREAM_H
