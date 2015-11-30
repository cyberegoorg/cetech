#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "celib/container/array.inl.h"

namespace cetech {

    /***************************************************************************
    **** Event stream interface
    ***************************************************************************/
    namespace eventstream {

        /***********************************************************************
        **** Event iterator
        ***********************************************************************/
        typedef uint32_t event_it;

        /***********************************************************************
        **** Event header struct
        ***********************************************************************/
        struct Header {
            uint32_t type;
            uint32_t size;
        };

        /***********************************************************************
        **** Remove all events
        ***********************************************************************/
        CE_INLINE void clear(EventStream& stream);

        /***********************************************************************
        **** Is stream empty?
        ***********************************************************************/
        CE_INLINE bool empty(EventStream& stream);

        /***********************************************************************
        **** Return stream size
        ***********************************************************************/
        CE_INLINE uint32_t size(EventStream& stream);

        /***********************************************************************
        **** Return event header
        ***********************************************************************/
        CE_INLINE Header* header(EventStream& stream,
                                 const event_it it);
        /***********************************************************************
        **** Return pointer to event
        ***********************************************************************/
        template < typename T >
        T* event(EventStream& stream,
                 const event_it it);

        /***********************************************************************
        **** Is iterator valid
        ***********************************************************************/
        CE_INLINE bool valid(EventStream& stream,
                             const event_it it);

        /***********************************************************************
        **** Get next iterator position
        ***********************************************************************/
        CE_INLINE event_it next(EventStream& stream,
                                const event_it it);

        /***********************************************************************
        **** Write events
        ***********************************************************************/
        CE_INLINE void write(EventStream& stream,
                             uint32_t type,
                             const void* events,
                             uint32_t size );

        CE_INLINE void write(EventStream& stream,
                             const void* events,
                             uint32_t size );

        /***********************************************************************
        **** Write event
        ***********************************************************************/
        template < typename T >
        void write(EventStream& stream,
                   uint32_t type,
                   T event );

    }

    /***************************************************************************
    **** Event stream implementation
    ***************************************************************************/
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

        Header* header(EventStream& stream,
                       const event_it it) {
            return (Header*)(&stream.stream[it]);
        }

        template < typename T >
        T* event(EventStream& stream,
                 const event_it it) {
            return (T*)(&stream.stream[it + sizeof(Header)]);
        }

        bool valid(EventStream& stream,
                   const event_it it) {
            return it < size(stream);
        }

        event_it next(EventStream& stream,
                      const event_it it) {
            return it + sizeof(Header) + header(stream, it)->size;
        }

        void write(EventStream& stream,
                   uint32_t type,
                   const void* events,
                   uint32_t size ) {
            Header header = {.type = type, .size = size};

            array::push( stream.stream, (char*)&header, sizeof(Header));
            array::push( stream.stream, (char*)events, (uint32_t) size);
        }

        void write(EventStream& stream,
                   const void* events,
                   uint32_t size ) {
            array::push( stream.stream, (char*)events, size);
        }

        template < typename T >
        void write(EventStream& stream,
                   uint32_t type,
                   T event ) {
            write(stream, type, &event, sizeof(T));
        }
    }

    /***************************************************************************
    **** Event stream implementation
    ***************************************************************************/
    inline EventStream::EventStream ( Allocator& allocator)  : stream(allocator) {}
};
