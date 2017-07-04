#ifndef CETECH_EVENTSTREAM_H
#define CETECH_EVENTSTREAM_H

//==============================================================================
// Includes
//==============================================================================

#include "array.inl"
#include "cetech/kernel/macros.h"

struct event_header {
    uint32_t type;
    uint64_t size;
};

namespace cetech {
    typedef Array<uint8_t> EventStream;

    namespace eventstream {
        template<typename T>
        inline void push(EventStream &stream,
                         uint32_t type,
                         const T &event) {
            event_header *header = (event_header *) (&event);

            header->type = type;
            header->size = sizeof(T);


            array::push(stream, (uint8_t *) header, sizeof(T));
        }

        inline event_header *begin(EventStream &stream) {
            return (event_header *) array::begin(stream);
        }

        inline event_header *end(EventStream &stream) {
            return (event_header *) array::end(stream);
        }

        inline event_header *next(EventStream &stream,
                                  event_header *header) {
            return (event_header *) (((char *) header) + header->size);
        }

        inline void clear(EventStream &stream) {
            array::clear(stream);
        }
    }

}


#endif //CETECH_EVENTSTREAM_H
