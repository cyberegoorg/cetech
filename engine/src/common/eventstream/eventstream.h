#pragma once

#include "common/memory/memory.h"
#include "common/container/hash.h"
#include "common/console_server/console_server.h"
#include "common/eventstream/eventstream_types.h"
#include <runtime/runtime.h>

#include "rapidjson/document.h"


namespace cetech {
    namespace eventstream {
        typedef uint32_t event_it;

        struct Header {
            uint32_t type;
            uint32_t size;
        };
        
        CE_INLINE void clear(EventStream& stream);
        CE_INLINE bool empty(EventStream& stream);
        CE_INLINE uint32_t size(EventStream& stream);

        Header* header(EventStream& stream, const event_it it);
        template < typename T >
        T* event(EventStream& stream, const event_it it);

        bool valid(EventStream& stream, const event_it it);
        event_it next(EventStream& stream, const event_it it);
        
        CE_INLINE void write(EventStream& stream, uint32_t type, const void* events, uint32_t size );
        template < typename T >
        void write(EventStream& stream, uint32_t type, T event );
        CE_INLINE void write(EventStream& stream, const void* events, uint32_t size );
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

        Header* header(EventStream& stream, const event_it it) {
            return (Header*)(&stream.stream[it]);
        }
        
        template < typename T >
        T* event(EventStream& stream, const event_it it) {
            return (T*)(&stream.stream[it + sizeof(Header)]);
        }

        bool valid(EventStream& stream, const event_it it) {
            return it < size(stream);
        }
        
        event_it next(EventStream& stream, const event_it it){
            return it + sizeof(Header) + header(stream, it)->size;
        }
        
        void write(EventStream& stream, uint32_t type, const void* events, uint32_t size ) {
            Header header = {.type = type, .size = size};

            array::push( stream.stream, (char*)&header, sizeof(Header));
            array::push( stream.stream, (char*)events, (size_t) size);
        }
        
        void write(EventStream& stream, const void* events, uint32_t size ) {
            array::push( stream.stream, (char*)events, size);
        }
        
        template < typename T >
        void write(EventStream& stream, uint32_t type, T event ) {
            write(stream, type, &event, sizeof(T));
        }
    }

    EventStream::EventStream ( Allocator& allocator)  : stream(allocator) {}
};
