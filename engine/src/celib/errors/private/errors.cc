#include "celib/errors/errors.h"
#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"

namespace cetech {
    namespace {
        static thread_local Array < const char* > * _scope_name = nullptr;
        static thread_local Array < const char* > * _scope_data = nullptr;
    };

    namespace error {
        const Array < const char* >& scope_name() {
            CE_ASSERT("error", _scope_name != nullptr);
            return *_scope_name;
        }

        const Array < const char* >& scope_data() {
            CE_ASSERT("error", _scope_data != nullptr);
            return *_scope_data;
        }

        void to_yaml(char* buffer) {
            // TODO: rewrite, safety
            const int size = array::size(*_scope_name);

            if (size == 0) {
                sprintf(buffer, " ~\n");
                return;
            }

            Array < const char* >& scope_name = *_scope_name;
            Array < const char* >& scope_data = *_scope_data;

            sprintf(buffer, "\n");

            char buff[256] = {0};
            for (int i = 0; i < size; ++i) {
                const char* name = scope_name[i];
                const char* data = scope_data[i];

                if (data) {
                    snprintf(buff, 4096, "    - %s: %s\n", name, data);
                } else {
                    snprintf(buff, 4096, "    - %s\n", name);
                }

                strcat(buffer, buff);
            }
        }

        void enter_scope(const char* name,
                         const char* data) {
            array::push_back(*_scope_name, name);
            array::push_back(*_scope_data, data);
        }

        void leave_scope() {
            array::pop_back(*_scope_name);
            array::pop_back(*_scope_data);
        }
    }

    namespace error_globals {
        void init() {
            _scope_name = MAKE_NEW(
                memory_globals::default_allocator(), Array < const char* >, memory_globals::default_allocator());
            _scope_data = MAKE_NEW(
                memory_globals::default_allocator(), Array < const char* >, memory_globals::default_allocator());
        }

        void shutdown() {
            MAKE_DELETE(memory_globals::default_allocator(), Array < const char* >, _scope_name);
            MAKE_DELETE(memory_globals::default_allocator(), Array < const char* >, _scope_data);
        }
    }
}
