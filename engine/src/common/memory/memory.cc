// based on bitsquid foundation
#include "memory.h"

#include <stdlib.h>
#include <assert.h>
#include <new>

namespace {
    using namespace cetech1;

    const uint32_t HEADER_PAD_VALUE = 0xffffffffu;

    struct Header {
        uint32_t size;
    };

    inline void* data_pointer(Header* header, uint32_t align) {
	CE_CHECK_PTR(header);
	
        void* p = header + 1;

        return memory::align_forward(p, align);
    }

    inline Header* header(void* data) {
	CE_CHECK_PTR(data);
      
        uint32_t* p = (uint32_t*)data;

        while (p[-1] == HEADER_PAD_VALUE) {
            --p;
        }

        return (Header*)p - 1;
    }

    inline void fill(Header* header, void* data, uint32_t size) {
        header->size = size;
        uint32_t* p = (uint32_t*)(header + 1);

        while (p < data) {
            *p++ = HEADER_PAD_VALUE;
        }
    }

    class MallocAllocator : public Allocator {
        public:
            MallocAllocator() : _total_allocated(0) {}

            ~MallocAllocator() {
		CE_ASSERT(_total_allocated == 0);
            }

            virtual void* allocate(uint32_t size, uint32_t align) {
                const uint32_t ts = size_with_padding(size, align);
                Header* h = (Header*)malloc(ts);
                void* p = data_pointer(h, align);
                fill(h, p, ts);
                _total_allocated += ts;
                return p;
            }

            virtual void deallocate(void* p) {
                if (!p) {
                    return;
                }

                Header* h = header(p);
                _total_allocated -= h->size;
                free(h);
            }

            virtual uint32_t allocated_size(void* p) {
	      CE_CHECK_PTR(p);
                return header(p)->size;
            }

            virtual uint32_t total_allocated() {
                return _total_allocated;
            }

        private:
            uint32_t _total_allocated;

            static inline uint32_t size_with_padding(uint32_t size, uint32_t align) {
                return size + align + sizeof(Header);
            }
    };


    struct MemoryGlobals {
        static const int ALLOCATOR_MEMORY = sizeof(MallocAllocator);
        char buffer[ALLOCATOR_MEMORY];

        MallocAllocator* default_allocator;

        MemoryGlobals() : default_allocator(0) {}
    };

    MemoryGlobals _memory_globals;
}

namespace cetech1 {
    namespace memory_globals {
        void init() {
            char* p = _memory_globals.buffer;

            _memory_globals.default_allocator = new(p) MallocAllocator();
        }


        void shutdown() {
            _memory_globals.default_allocator->~MallocAllocator();
            _memory_globals = MemoryGlobals();
        }

        Allocator& default_allocator() {
            return *_memory_globals.default_allocator;
        }

    }

}