#pragma once

#include "common/memory/memory.h"
#include "common/container/hash.h"
#include "common/console_server/console_server.h"
#include "common/eventstream/eventstream_types.h"

#include "rapidjson/document.h"


namespace cetech {
    namespace eventstream {
        CE_INLINE void clear(EventStream& stream);

        CE_INLINE EventStreamHeader* header(EventStream& stream, event_it event);
        template < typename T > T* data(EventStream& stream, event_it event);

        CE_INLINE bool empty(EventStream& stream);

        CE_INLINE event_it first(EventStream& stream);
        CE_INLINE event_it next(EventStream& stream, event_it event);

        template < typename T > T* prepare_new(EventStream& stream, uint32_t type);

        CE_INLINE void add_events(EventStream& stream, event_t* events, uint32_t size);
    }

    
    namespace eventstream {
        void clear(EventStream& stream) {
            array::clear(stream.stream);
        }

        EventStreamHeader* header(EventStream& stream, event_it event) {
            return (EventStreamHeader*)(array::begin(stream.stream) + event);
        }

        template < typename T >
        T* data(EventStream& stream, event_it event) {
            return (T*)(&(header(stream, event)->data));
        }

        event_it first(EventStream& stream) {
            return 0;
        }

        event_it end(EventStream& stream) {
            return array::size(stream.stream);
        }
        
        bool empty(EventStream& stream) {
            return array::empty(stream.stream);
        }
        
        event_it next(EventStream& stream, event_it event) {
            const uint32_t sz = header(stream, event)->size;
            event_it n = event + sizeof(EventStreamHeader) + sz;

            return n;
        }

        template < typename T >
        T* prepare_new(EventStream& stream, uint32_t type) {
            event_it it = end(stream);
            array::resize(stream.stream, array::size(stream.stream) + sizeof(EventStreamHeader) + sizeof(T));
            
            char *p = array::begin(stream.stream) + it;
            
            EventStreamHeader* h = (EventStreamHeader*)p;
            h->type = type;
            h->size = sizeof(T);

            return (T*)(&(h->data));
        }

        void add_events(EventStream& stream, event_t* events, uint32_t size) {
            event_t* first = array::end(stream.stream);
            array::resize(stream.stream, array::size(stream.stream) + size);

            memcpy(first, events, size);
        };
    }

    EventStream::EventStream ( Allocator& allocator )  : stream(allocator) {
        array::reserve(stream, 1024);
    }
};
