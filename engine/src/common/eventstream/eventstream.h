#pragma once

#include "common/memory/memory.h"
#include "common/container/hash.h"
#include "common/console_server/console_server.h"
#include "common/eventstream/eventstream_types.h"
#include <runtime/runtime.h>

#include "rapidjson/document.h"


namespace cetech {
    namespace eventstream {
        CE_INLINE void clear(EventStream& stream);

        CE_INLINE bool empty(EventStream& stream);

    }


    namespace eventstream {
        void clear(EventStream& stream) {
            array::clear(stream.stream);
        }

        bool empty(EventStream& stream) {
            return array::empty(stream.stream);
        }

        uint32_t size(EventStream& stream) {
            return array::size(stream.stream);
        }

        void write(EventStream& stream, uint32_t type, const void* events, uint32_t size ) {
            EventStreamHeader header = {.type = type, .size = size};

            array::push( stream.stream, (char*)&header, sizeof(EventStreamHeader));
            array::push( stream.stream, (char*)events, (size_t) size);
        }

        template < typename T >
        void write(EventStream& stream, uint32_t type, T event ) {
            write(stream, type, &event, sizeof(T));
        }

        void write(EventStream& stream, const void* events, uint32_t size ) {
            array::push( stream.stream, (char*)events, size);
        }
    }

    EventStream::EventStream ( Allocator& allocator)  : stream(allocator) {}
};
