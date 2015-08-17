// based on bitsquid foundation
#pragma once

#include <cstring>

#include "common/asserts.h"
#include "common/types.h"

namespace cetech1 {

    /*! Abstract allocator.
     */
    class Allocator {
        public:
            static const uint32_t DEFAULT_ALIGN = 4;              //!< Default align.
            static const uint32_t SIZE_NOT_TRACKED = 0xffffffffu; //!< Size not tracked

        public:
            Allocator() {};
            virtual ~Allocator() {};

        public:
            /*! Allocate memory.
             * \param size Alloc size.
             * \param align align.
             * \return Pointer to new memory.
             */
            virtual void* allocate(const uint32_t size, const uint32_t align = DEFAULT_ALIGN) = 0;

            /*! Deallocate memory.
             * \param p Pointer to memory.
             */
            virtual void deallocate(void* p) = 0;

        public:
            /*! Get allocated size of memory.
             * \param p Pointer to memory.
             * \return Allocated size or SIZE_NOT_TRACKED if allocator does not track size.
             */
            virtual uint32_t allocated_size(void* p) = 0;

            /*! Total allocated memory.
             * \return Total allocated memory or SIZE_NOT_TRACKED if allocator does not track size.
             */
            virtual uint32_t total_allocated() = 0;

        private:
            Allocator(const Allocator &other) {
                (void)other;
            };
            Allocator& operator = (const Allocator &other) {
                (void)other; return *this;
            };
    };

    namespace memory_globals {
        /*! Init global memory.
         */
        void init();

        /*! Shutdown global memory.
         */
        void shutdown();

        /*! Get default allocator.
         * \return Default allocator.
         */
        Allocator& default_allocator();
    }

    namespace memory {
        /*! Allign pointer.
         * \param p Pointer.
         * \param align Align.
         * \return Alligned pointer.
         */
        FORCE_INLINE void* align_forward(void* p, const uint32_t align);

        FORCE_INLINE void* pointer_add(void* p, const uint32_t bytes);
        FORCE_INLINE const void* pointer_add(const void* p, const uint32_t bytes);

        FORCE_INLINE void* pointer_sub(void* p, const uint32_t bytes);
        FORCE_INLINE const void* pointer_sub(const void* p, const uint32_t bytes);

        FORCE_INLINE void* memcpy(void* dst, const void* src, const uint32_t bytes);
    }

    FORCE_INLINE void* memory::align_forward(void* p, const uint32_t align) {
        uintptr_t pi = uintptr_t(p);
        const uint32_t mod = pi % align;

        if (mod) {
            pi += (align - mod);
        }

        return (void*)pi;
    }

    FORCE_INLINE void* memory::pointer_add(void* p, const uint32_t bytes) {
        CE_CHECK_PTR(p);

        return (void*)((char*)p + bytes);
    }

    FORCE_INLINE const void* memory::pointer_add(const void* p, const uint32_t bytes) {
        CE_CHECK_PTR(p);

        return (const void*)((const char*)p + bytes);
    }

    FORCE_INLINE void* memory::pointer_sub(void* p, const uint32_t bytes) {
        CE_CHECK_PTR(p);

        return (void*)((char*)p - bytes);
    }

    FORCE_INLINE const void* memory::pointer_sub(const void* p, const uint32_t bytes) {
        CE_CHECK_PTR(p);
        return (const void*)((const char*)p - bytes);
    }

    FORCE_INLINE void* memory::memcpy(void* dst, const void* src, const uint32_t bytes) {
        CE_CHECK_PTR(dst);
        CE_CHECK_PTR(src);
        return std::memcpy(dst, src, bytes);
    }
}
