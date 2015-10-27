// based on bitsquid foundation
#include "memory.h"
#include <celib/container/hash.inl.h>

#include <stdlib.h>
#include <assert.h>

#include <execinfo.h>
#include <cxxabi.h>

static char* c_stacktrace( int skip = 2) {
    char* return_str = (char*)malloc(4096 * 8);
    return_str[0] = '\0';

    void* array[50];
    int size = backtrace(array, 50);

    char** messages = backtrace_symbols(array, size);

    for (int i = skip; i < size && messages != NULL; ++i) {
        char* mangled_name = 0, * offset_begin = 0, * offset_end = 0;

        for (char* p = messages[i]; *p; ++p) {
            if (*p == '(') {
                mangled_name = p;
            } else if (*p == '+') {
                offset_begin = p;
            } else if (*p == ')') {
                offset_end = p;
                break;
            }
        }

        if (mangled_name && offset_begin && offset_end && mangled_name < offset_begin) {
            *mangled_name++ = '\0';
            *offset_begin++ = '\0';
            *offset_end++ = '\0';

            int status;
            char* real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);

            if (status == 0) {
                char buffer[4096] = {0};
                sprintf(buffer,
                        "\t[%d] %s: (%s)+%s %s\n",
                        i,
                        messages[i],
                        (status == 0 ? real_name : mangled_name),
                        offset_begin,
                        offset_end);
                strcat(return_str, buffer);
                free(real_name);
            }
        } else {
            char buffer[4096] = {0};
            sprintf(buffer, "\t[%d] %s\n", i, messages[i]);
            strcat(return_str, buffer);
        }
    }

    free(messages);

    return return_str;
}

namespace {
    using namespace cetech;

    const uint32_t HEADER_PAD_VALUE = 0xffffffffu;

    struct Header {
        uint32_t size;
        const char* tag;
    };

    inline void* data_pointer(Header* header, const uint32_t align) {
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

    inline void fill(Header* header, void* data, const uint32_t size) {
        header->size = size;
        uint32_t* p = (uint32_t*)(header + 1);

        while (p < data) {
            *p++ = HEADER_PAD_VALUE;
        }
    }

    class MallocAllocator : public Allocator {
        public:
            enum {
                MAX_MEM_TRACE = 4096
            };

            struct TraceEntry {
                char used : 1;
                void* p;
                const char* traceback;
            } mem_trace[MAX_MEM_TRACE];

            void add_entry(void* p, const char* traceback) {
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
                }
            }

            MallocAllocator() : _total_allocated(0) {
                memset(mem_trace, 0, sizeof(TraceEntry) * 4096);
            }

            ~MallocAllocator() {
                for (int i = 0; i < 4096; ++i) {
                    if (!mem_trace[i].used) {
                        continue;
                    }

                    printf("Undealocated memory :\n%s", mem_trace[i].traceback);
                }

                CE_ASSERT(_total_allocated == 0);
            }

            virtual void* allocate(const uint32_t size, const uint32_t align) {
                //                printf("allocate: %s\n", tag);
                const uint32_t ts = size_with_padding(size, align);
                Header* h = (Header*)memory::malloc(ts);
                void* p = data_pointer(h, align);
                fill(h, p, ts);
                _total_allocated += ts;


                const char* trace = c_stacktrace();
                add_entry(p, trace);
                return p;
            }

            virtual void deallocate(void* p) {
                if (!p) {
                    return;
                }

                Header* h = header(p);
                _total_allocated -= h->size;

                remove_entry(p);
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

            static inline uint32_t size_with_padding(const uint32_t size, const uint32_t align) {
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