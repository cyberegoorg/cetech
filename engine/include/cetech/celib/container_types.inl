#ifndef CETECH_CONTAINER_TYPES_H
#define CETECH_CONTAINER_TYPES_H

#include <cstdint>

struct ct_allocator;

namespace cetech {
    template<typename T>
    struct Array {
        Array();

        Array(ct_allocator *a);

        ~Array();

        Array(const Array &other);

        Array &operator=(const Array &other);

        T &operator[](uint32_t i);

        const T &operator[](uint32_t i) const;

        void init(ct_allocator *a);

        void destroy();

        ct_allocator *_allocator;
        uint32_t _size;
        uint32_t _capacity;
        T *_data;
    };

    template<typename T>
    struct Map {
    public:
        Map();

        Map(ct_allocator *a);

        void init(ct_allocator *a);

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

        explicit Queue(ct_allocator *a);

        T &operator[](const uint32_t i);

        const T &operator[](const uint32_t i) const;

        void init(ct_allocator *a);

        void destroy();

        Array<T> _data; // Data
        uint32_t _size;    // Size
        uint32_t _offset;  // Offset
    };

}

#endif //CETECH_CONTAINER_TYPES_H
