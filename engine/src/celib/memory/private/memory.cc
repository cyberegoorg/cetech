/*******************************************************************************
**** Based on bitsquid foundation.
*******************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <cstdint>

#include "celib/memory/memory.h"
#include "celib/stacktrace/stacktrace.h"



namespace {
    using namespace cetech;

    const uint32_t HEADER_PAD_VALUE = 0xffffffffu;

    struct Header {
        uint32_t size;
        const char* tag;
    };

    inline void* data_pointer(Header* header,
                              const uint32_t align) {
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

    inline void fill(Header* header,
                     void* data,
                     const uint32_t size) {
        header->size = size;
        uint32_t* p = (uint32_t*)(header + 1);

        while (p < data) {
            *p++ = HEADER_PAD_VALUE;
        }
    }

    class MallocAllocator : public Allocator {
        public:
#if defined(CETECH_DEBUG)
            enum {
                MAX_MEM_TRACE = 4096
            };

            struct TraceEntry {
                char used : 1;
                void* p;
                char* traceback;
            } mem_trace[MAX_MEM_TRACE];

            void add_entry(void* p,
                           const char* traceback) {
                for (int i = 0; i < MAX_MEM_TRACE; ++i) {
                    if (!mem_trace[i].used) {
                        mem_trace[i].used = 1;
                        mem_trace[i].p = p;
                        mem_trace[i].traceback = strdup(traceback);
                        break;
                    }
                }
            }

            void remove_entry(void* p) {
                for (int i = 0; i < MAX_MEM_TRACE; ++i) {
                    if (mem_trace[i].p != p) {
                        continue;
                    }

                    mem_trace[i].used = 0;
                    free(mem_trace[i].traceback);
                    mem_trace[i].traceback = 0;
                }
            }
#endif

            MallocAllocator() : _total_allocated(0) {
#if defined(CETECH_DEBUG)
                memset(mem_trace, 0, sizeof(TraceEntry) * MAX_MEM_TRACE);
#endif
            }

            ~MallocAllocator() {
#if defined(CETECH_DEBUG)
                char ok = 1;
                for (int i = 0; i < 4096; ++i) {
                    if (!mem_trace[i].used) {
                        continue;
                    }

                    ok = 0;

                    log::error("memory", "LEAK: %p\n    traceback:\n%s", mem_trace[i].p,
                               mem_trace[i].traceback);
                    deallocate(mem_trace[i].p);
                }

                for (int i = 0; i < 4096; ++i) {
                    if (mem_trace[i].traceback != 0) {
                        free(mem_trace[i].traceback);
                    }
                }

                CE_ASSERT(ok);
#endif
                CE_ASSERT(_total_allocated == 0);

            }

            virtual void* allocate(const uint32_t size,
                                   const uint32_t align) {
                //                printf("allocate: %s\n", tag);
                const uint32_t ts = size_with_padding(size, align);
                Header* h = (Header*)memory::malloc(ts);
                void* p = data_pointer(h, align);
                fill(h, p, ts);
                _total_allocated += ts;

#if defined(CETECH_DEBUG)
                const char* trace = stacktrace(2);
                add_entry(p, trace);
#endif
                return p;
            }

            virtual void deallocate(void* p) {
                if (!p) {
                    return;
                }

                Header* h = header(p);
                _total_allocated -= h->size;

#if defined(CETECH_DEBUG)
                remove_entry(p);
#endif

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

            static inline uint32_t size_with_padding(const uint32_t size,
                                                     const uint32_t align) {
                return size + align + sizeof(Header);
            }
    };


    struct MemoryGlobals {
        static const int ALLOCATOR_MEMORY = sizeof(MallocAllocator);
        char buffer[ALLOCATOR_MEMORY];

        MallocAllocator* default_allocator;

        MemoryGlobals() : buffer {
            0
        }, default_allocator(0) {}
    };

    MemoryGlobals _memory_globals;
}

namespace cetech {
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
