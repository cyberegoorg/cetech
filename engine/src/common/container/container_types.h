// based on bitsquid foundation
#pragma once

#include "common/types.h"
#include "common/memory/memory_types.h"

namespace cetech1 {

/*! Array container.
 */
    template < typename T > struct Array {
        /*! Create array.
         * \param a Allocator.
         */
        Array(Allocator & a);

        /*! Create array from other array.
         * \param other Other array.
         */
        Array(const Array &other);

        /*! Destruct array.
         */
        ~Array();

        /*! Copy array to this array.
         * \param other Other array.
         */
        Array& operator = (const Array &other);

        /*! Item accessor.
         */
        T& operator[] (uint32_t i);

        /*! Item const accessor.
         */
        const T& operator[] (uint32_t i) const;


        Allocator* _allocator; //!< Pointer to allocator.
        uint32_t _size;        //!< Size
        uint32_t _capacity;    //!< Allocate size.
        T* _data;              //!< Array data;
    };


/*! Hash map.
 */
    template < typename T > struct Hash {
        /*! Create hash map
         * \param a Allocator
         */
        Hash(Allocator & a);

        /*! Destroy hash map
         */
        ~Hash();

        /*! Hash map entry.
         */
        struct Entry {
            uint64_t key;  //!< Key.
            uint32_t next; //!< Next entry index.
            T value;       //!< Entry value.
        };

        Array < uint32_t > _hash;    //!< Key hash -> Data index.
        Array < Entry > _data;       //!< Data.
    };

/*!
 * A double-ended queue/ring buffer.
 */
    template < typename T > struct Queue {
        /*! Create queue
         * \param a Allocator
         */
        Queue(Allocator & a);

        T& operator[] (uint32_t i);
        const T& operator[] (uint32_t i) const;

        Array < T > _data;      //!< Data
        uint32_t _size;         //!< Size
        uint32_t _offset;       //!< Offset
    };
}