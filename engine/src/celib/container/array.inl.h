/*******************************************************************************
**** Based on bitsquid foundation.
*******************************************************************************/

#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include <memory.h>
#include <string.h>
#include <type_traits>


#include "celib/container_types.h"
#include "celib/memory/memory.h"

namespace cetech {

    /***************************************************************************
    **** Array interface
    ***************************************************************************/
    namespace array {

        /***********************************************************************
        **** Return array size
        ***********************************************************************/
        template < typename T >
        uint32_t size(const Array < T >& a);

        /***********************************************************************
        **** Has array any items?
        ***********************************************************************/
        template < typename T >
        bool any(const Array < T >& a);

        /***********************************************************************
        **** Is array empty?
        ***********************************************************************/
        template < typename T >
        bool empty(const Array < T >& a);


        /***********************************************************************
        **** Return pointer to begin
        ***********************************************************************/
        template < typename T >
        T* begin(Array < T >& a);

        /***********************************************************************
        **** Return const pointer to begin
        ***********************************************************************/
        template < typename T >
        const T* begin(const Array < T >& a);

        /***********************************************************************
        **** Return pointer to end
        ***********************************************************************/
        template < typename T >
        T* end(Array < T >& a);

        /***********************************************************************
        **** Return const pointer to end
        ***********************************************************************/
        template < typename T >
        const T* end(const Array < T >& a);


        /***********************************************************************
        **** Return first item
        ***********************************************************************/
        template < typename T >
        T& front(Array < T >& a);

        /***********************************************************************
        **** Return const first item
        ***********************************************************************/
        template < typename T >
        const T& front(const Array < T >& a);

        /***********************************************************************
        **** Return last item
        ***********************************************************************/
        template < typename T >
        T& back(Array < T >& a);

        /***********************************************************************
        **** Return const last item
        ***********************************************************************/
        template < typename T >
        const T& back(const Array < T >& a);


        /***********************************************************************
        **** Resize array to new_size
        ***********************************************************************/
        template < typename T >
        void resize(Array < T >& a,
                    const uint32_t new_size);

        /***********************************************************************
        **** Remove all items from array
        ***********************************************************************/
        template < typename T >
        void clear(Array < T >& a);

        /***********************************************************************
        **** Realloc array to specified capacity.
        ***********************************************************************/
        template < typename T >
        void set_capacity(Array < T >& a,
                          const uint32_t new_capacity);

        /***********************************************************************
        **** Reserve capacity.
        ***********************************************************************/
        template < typename T >
        void reserve(Array < T >& a,
                     const uint32_t new_capacity);

        /***********************************************************************
        **** Grow array using geometric grow facor.
        ***********************************************************************/
        template < typename T >
        void grow(Array < T >& a,
                  const uint32_t min_capacity = 0);

        /***********************************************************************
        **** Trim array.
        ***********************************************************************/
        template < typename T >
        void trim(Array < T >& a);

        /***********************************************************************
        **** Push count items to array.
        ***********************************************************************/
        template < typename T >
        void push(Array < T >& a,
                  const T* items,
                  uint32_t count);

        /***********************************************************************
        **** Push item to end
        ***********************************************************************/
        template < typename T >
        void push_back(Array < T >& a,
                       const T& item);

        /***********************************************************************
        **** Pop last item
        ***********************************************************************/
        template < typename T >
        void pop_back(Array < T >& a);
    }


    /***************************************************************************
    **** Array implementation
    ***************************************************************************/
    namespace array {
        template < typename T >
        inline uint32_t size(const Array < T >& a) {
            return a._size;
        }

        template < typename T >
        inline bool any(const Array < T >& a) {
            return a._size != 0;
        }

        template < typename T >
        inline bool empty(const Array < T >& a) {
            return a._size == 0;
        }


        template < typename T >
        inline T* begin(Array < T >& a) {
            return a._data;
        }


        template < typename T >
        inline const T* begin(const Array < T >& a) {
            return a._data;
        }

        template < typename T >
        inline T* end(Array < T >& a) {
            return a._data + a._size;
        }

        template < typename T >
        inline const T* end(const Array < T >& a) {
            return a._data + a._size;
        }


        template < typename T >
        inline T& front(Array < T >& a) {
            return a._data[0];
        }

        template < typename T >
        inline const T& front(const Array < T >& a) {
            return a._data[0];
        }

        template < typename T >
        inline T& back(Array < T >& a) {
            return a._data[a._size - 1];
        }

        template < typename T >
        inline const T& back(const Array < T >& a) {
            return a._data[a._size - 1];
        }


        template < typename T >
        inline void resize(Array < T >& a,
                           const uint32_t new_size) {
            if (new_size > a._capacity) {
                grow(a, new_size);
            }

            a._size = new_size;
        }

        template < typename T >
        inline void clear(Array < T >& a) {
            resize(a, 0);
        }


        template < typename T >
        inline void     set_capacity(Array < T >& a,
                                     const uint32_t new_capacity) {
            CE_CHECK_PTR(a._allocator);

            if (new_capacity == a._capacity) {
                return;
            }

            if (new_capacity < a._size) {
                resize(a, new_capacity);
            }

            T* new_data = 0;
            if (new_capacity > 0) {
                new_data = (T*)a._allocator->allocate(sizeof(T) * new_capacity, __alignof(T));
                memory::memcpy(new_data, a._data, sizeof(T) * a._size);
            }

            a._allocator->deallocate(a._data);
            a._data = new_data;
            a._capacity = new_capacity;
        }



        template < typename T >
        inline void reserve(Array < T >& a,
                            const uint32_t new_capacity) {
            if (new_capacity > a._capacity) {
                set_capacity(a, new_capacity);
            }
        }

        template < typename T >
        inline void grow(Array < T >& a,
                         const uint32_t min_capacity) {
            uint32_t new_capacity = a._capacity * 2 + 8;

            if (new_capacity < min_capacity) {
                new_capacity = min_capacity;
            }

            set_capacity(a, new_capacity);
        }


        template < typename T >
        inline void trim(Array < T >& a) {
            set_capacity(a, a._size);
        }

        template < typename T >
        void push(Array < T >& a,
                  const T* items,
                  uint32_t count) {
            CE_ASSERT(std::is_pod < T > ());

            if (a._capacity <= a._size + count) {
                grow(a, a._size + count);
            }

            memcpy(&a._data[a._size], items, sizeof(T) * count);
            a._size += count;
        }

        template < typename T >
        inline void push_back(Array < T >& a,
                              const T& item) {
            CE_ASSERT(std::is_pod < T > ());

            if (a._size + 1 > a._capacity) {
                grow(a);
            }

            a._data[a._size++] = item;
        }

        template < typename T >
        inline void pop_back(Array < T >& a) {
            a._size--;
        }

    }

    /***************************************************************************
    **** Array implementation
    ***************************************************************************/
    template < typename T >
    inline Array < T > ::Array(Allocator & a) : _allocator(&a), _size(0), _capacity(0),
                                                _data(nullptr) {}

    template < typename T >
    inline Array < T > ::Array(const Array &other) : _allocator(other._allocator), _size(0), _capacity(0),
                                                     _data(nullptr) {
        const uint32_t n = other._size;
        array::set_capacity(*this, n);

        memory::memcpy(_data, other._data, sizeof(T) * n);

        _size = n;
    }

    template < typename T >
    inline Array < T > ::~Array() {
        _allocator->deallocate(_data);
    }

    template < typename T >
    inline Array < T >& Array < T > ::operator = (const Array &other) {
        const uint32_t n = other._size;
        array::resize(*this, n);
        memory::memcpy(_data, other._data, sizeof(T) * n);
        return *this;
    }

    template < typename T >
    inline T &Array < T > ::operator [] (const uint32_t i) {
        return _data[i];
    }

    template < typename T >
    inline const T &Array < T > ::operator [] (const uint32_t i) const {
        return _data[i];
    }
}
