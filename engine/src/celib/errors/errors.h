#pragma once

#include "celib/container/types.h"

namespace cetech {
    namespace error {
        void enter_scope(const char* name,
                         const char* data);
        void leave_scope();

        const Array < const char* >& scope_name();
        const Array < const char* >& scope_data();

        void to_yaml(char* buffer);

        struct ErrorScope {
            ErrorScope(const char* name, const char* data) {
                error::enter_scope(name, data);
            }

            ~ErrorScope() {
                error::leave_scope();
            }
        };
    }

    namespace error_globals {
        void init();
        void shutdown();
    }
}
