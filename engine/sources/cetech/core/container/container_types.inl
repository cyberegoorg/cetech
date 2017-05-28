#ifndef CETECH_CONTAINER_TYPES_H
#define CETECH_CONTAINER_TYPES_H

#include <cstdint>

struct allocator;

namespace cetech {
    template<typename T>
    struct Array {
        Array();

        Array(allocator *a);

        ~Array();

        Array(const Array &other);

        Array &operator=(const Array &other);

        T &operator[](uint32_t i);

        const T &operator[](uint32_t i) const;

        void init(allocator *a);

        void destroy();

        allocator *_allocator;
        uint32_t _size;
        uint32_t _capacity;
        T *_data;
    };

    template<typename T>
    struct Map {
    public:
        Map();

        Map(allocator *a);

        void init(allocator *a);

        void destroy();

        struct Entry {
            uint64_t key;
            uint32_t next;
            T value;
        };

        Array<uint32_t> _hash;
        Array<Entry> _data;
    };
}

#endif //CETECH_CONTAINER_TYPES_H
