#ifndef CELIB_CONTAINER_TYPES_H
#define CELIB_CONTAINER_TYPES_H

#include <cstdint>

struct cel_alloc;

namespace celib {
    template<typename T>
    struct Array {
        Array();

        Array(cel_alloc *a);

        ~Array();

        Array(const Array &other);

        Array &operator=(const Array &other);

        T &operator[](uint32_t i);

        const T &operator[](uint32_t i) const;

        void init(cel_alloc *a);

        void destroy();

        cel_alloc *_allocator;
        uint32_t _size;
        uint32_t _capacity;
        T *_data;
    };

    template<typename T>
    struct Map {
    public:
        Map();

        Map(cel_alloc *a);

        void init(cel_alloc *a);

        void destroy();

        struct Entry {
            uint64_t key;
            uint32_t next;
            T value;
        };

        Array<uint32_t> _hash;
        Array<Entry> _data;
    };

    /***************************************************************************
    **** A double-ended queue/ring buffer
    ***************************************************************************/
    template<typename T>
    struct Queue {
        Queue();

        explicit Queue(cel_alloc *a);

        T &operator[](const uint32_t i);

        const T &operator[](const uint32_t i) const;

        void init(cel_alloc *a);

        void destroy();

        Array<T> _data; // Data
        uint32_t _size;    // Size
        uint32_t _offset;  // Offset
    };

}

#endif //CELIB_CONTAINER_TYPES_H
