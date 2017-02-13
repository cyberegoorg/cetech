#ifndef CELIB_EVENTSTREAM_H
#define CELIB_EVENTSTREAM_H

//==============================================================================
// Includes
//==============================================================================

#include "array.h"
#include "../types.h"

struct event_header {
    u32 type;
    u64 size;
};

struct eventstream {
    struct array_u8 stream;
};

//==============================================================================
// Interface
//==============================================================================

static inline void eventstream_create(struct eventstream *es,
                                      struct cel_allocator *allocator) {
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

static inline void *_eventstream_push(struct eventstream *es,
                                      struct event_header *header,
                                      u32 type,
                                      u64 size) {
    header->type = type;
    header->size = size;
    size_t s = ARRAY_SIZE(&es->stream);
    array_push_u8(&es->stream, (u8 *) header, size);

    return (void *) (&ARRAY_AT(&es->stream, s));
}

#endif //CELIB_EVENTSTREAM_H
