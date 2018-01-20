#ifndef CECORE_CONTAINER_TYPES_H
#define CECORE_CONTAINER_TYPES_H

#include <cstdint>

struct ct_alloc;

namespace celib {
    template<typename T>
    struct Array {
        Array();

        Array(ct_alloc *a);

        ~Array();

        Array(const Array &other);

        Array &operator=(const Array &other);

        T &operator[](uint32_t i);

        const T &operator[](uint32_t i) const;

        void init(ct_alloc *a);

        void destroy();

        ct_alloc *_allocator;
        uint32_t _size;
        uint32_t _capacity;
        T *_data;
    };

    template<typename T>
    struct Map {
    public:
        Map();

        Map(ct_alloc *a);

        void init(ct_alloc *a);

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

#endif //CECORE_CONTAINER_TYPES_H
