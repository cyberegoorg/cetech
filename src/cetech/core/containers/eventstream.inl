#ifndef CECORE_EVENTSTREAM_H
#define CECORE_EVENTSTREAM_H

//==============================================================================
// Includes
//==============================================================================

#include "array.inl"
#include "array.h"

namespace celib {
    typedef Array <uint8_t> EventStream;

    namespace eventstream {
        template<typename H, typename T>
        inline void push(EventStream &stream,
                         uint32_t type,
                         const T &event) {
            H *header = (H *) (&event);

            header->type = type;
            header->size = sizeof(T);


            array::push(stream, (uint8_t *) header, sizeof(T));
        }

        template<typename H>
        inline H *begin(EventStream &stream) {
            return (H *) array::begin(stream);
        }

        template<typename H>
        inline H *end(EventStream &stream) {
            return (H *) array::end(stream);
        }

        template<typename H>
        inline H *next(H *header) {
            return (H *) (((char *) header) + header->size);
        }

        inline void clear(EventStream &stream) {
            array::clear(stream);
        }
    }

}


#endif //CECORE_EVENTSTREAM_H
