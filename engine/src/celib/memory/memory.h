#pragma once

/*******************************************************************************
**** Based on bitsquid foundation.
*******************************************************************************/


/*******************************************************************************
**** Includes
*******************************************************************************/

#include <cstring>
#include <new>

#include "celib/asserts.h"
#include "celib/types.h"


/***************************************************************************
**** allocate T(...) with 'a' alocator
***************************************************************************/
#define MAKE_NEW(a, T, ...) (new((a).allocate(sizeof(T), alignof(T))) T(__VA_ARGS__))

/***************************************************************************
**** deallocate T* p with 'a' alocator
***************************************************************************/
#define MAKE_DELETE(a, T, p)    do {if (p) {(p)->~T(); a.deallocate(p); }} while (0)

namespace cetech {

    /***************************************************************************
    **** Abstract allocator
    ***************************************************************************/
    class Allocator {
        public:

            /*******************************************************************
            **** Default align.
            *******************************************************************/
            static const uint32_t DEFAULT_ALIGN = 4;

            /*******************************************************************
            **** Size not tracked
            *******************************************************************/
            static const uint32_t SIZE_NOT_TRACKED = 0xffffffffu;

        public:
            /*******************************************************************
            **** Constructor and destructor
            *******************************************************************/
            Allocator() {};
            virtual ~Allocator() {};

        public:

            /*******************************************************************
            **** Allocate memory.
            *******************************************************************/
            virtual void* allocate(const uint32_t size,
                                   const uint32_t align = DEFAULT_ALIGN) = 0;

            /*******************************************************************
            **** Deallocate memory.
            *******************************************************************/
            virtual void deallocate(void* p) = 0;

        public:
            /*******************************************************************
            **** Get allocated size of memory or SIZE_NOT_TRACKED if allocator
            **** does not track size.
            *******************************************************************/
            virtual uint32_t allocated_size(void* p) = 0;

            /*******************************************************************
            **** Total allocated memory or SIZE_NOT_TRACKED if allocator
            **** does not track size
            *******************************************************************/
            virtual uint32_t total_allocated() = 0;

        private:
            explicit Allocator(const Allocator& other) {
                (void)other;
            };
            Allocator& operator = (const Allocator &other) {
                (void)other; return *this;
            };
    };


    /***************************************************************************
    **** Memory system globals function.
    ***************************************************************************/
    namespace memory_globals {

        /***********************************************************************
        **** Init system.
        ***********************************************************************/
        void init();

        /***********************************************************************
        **** Shutdown system.
        ***********************************************************************/
        void shutdown();

        /***********************************************************************
        **** Get default allocator.
        ***********************************************************************/
        Allocator& default_allocator();
    }

    /***************************************************************************
    **** Memory interface
    ***************************************************************************/
    namespace memory {

        /***********************************************************************
        **** Allocate array system globals function.
        ***********************************************************************/
        template < typename T, typename ... ARGS >
        CE_INLINE T* alloc_array(Allocator& allocator,
                                 const size_t size,
                                 ARGS ... args) {
            T* mem = (T*) allocator.allocate(sizeof(T) * size, alignof(T));
            char* p = (char*)mem;

            for (size_t i = 0; i < size; ++i) {
                new(p) T(args ...);
                p += sizeof(T);
            }

            return mem;
        }

        /***********************************************************************
        **** Allign pointer.
        ***********************************************************************/
        CE_INLINE void* align_forward(void* p,
                                      const uint32_t align);

        /***********************************************************************
        **** Return pointer + bytes
        ***********************************************************************/
        CE_INLINE void* pointer_add(void* p,
                                    const uint32_t bytes);

        /***********************************************************************
        **** Return pointer + bytes
        ***********************************************************************/
        CE_INLINE const void* pointer_add(const void* p,
                                          const uint32_t bytes);

        /***********************************************************************
        **** Return pointer - bytes
        ***********************************************************************/
        CE_INLINE void* pointer_sub(void* p,
                                    const uint32_t bytes);

        /***********************************************************************
        **** Return pointer - bytes
        ***********************************************************************/
        CE_INLINE const void* pointer_sub(const void* p,
                                          const uint32_t bytes);

        /***********************************************************************
        **** Memcpy
        ***********************************************************************/
        CE_INLINE void* memcpy(void* dst,
                               const void* src,
                               const uint32_t bytes);

        /***********************************************************************
        **** Malloc
        ***********************************************************************/
        CE_INLINE void* malloc(const uint32_t bytes);
    }

    /***************************************************************************
    **** Memory implementation
    ***************************************************************************/
    void* memory::align_forward(void* p,
                                const uint32_t align) {
        uintptr_t pi = uintptr_t(p);
        const uint32_t mod = pi % align;

        if (mod) {
            pi += (align - mod);
        }

        return (void*)pi;
    }

    void* memory::pointer_add(void* p,
                              const uint32_t bytes) {
        CE_ASSERT("memory", p != nullptr);

        return (void*)((char*)p + bytes);
    }

    const void* memory::pointer_add(const void* p,
                                    const uint32_t bytes) {
        CE_ASSERT("memory", p != nullptr);
        return (const void*)((const char*)p + bytes);
    }

    void* memory::pointer_sub(void* p,
                              const uint32_t bytes) {
        CE_ASSERT("memory", p != nullptr);
        return (void*)((char*)p - bytes);
    }

    const void* memory::pointer_sub(const void* p,
                                    const uint32_t bytes) {
        CE_ASSERT("memory", p != nullptr);
        return (const void*)((const char*)p - bytes);
    }

    void* memory::memcpy(void* dst,
                         const void* src,
                         const uint32_t bytes) {
        CE_ASSERT("memory", dst != nullptr);
        return std::memcpy(dst, src, bytes);
    }

    void* memory::malloc(const uint32_t bytes) {
        void* res = std::malloc(bytes);
        CE_ASSERT("memory", res != nullptr);
        return res;
    }
}
