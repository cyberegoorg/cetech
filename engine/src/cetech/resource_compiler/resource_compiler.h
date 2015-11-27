#pragma once

#include <cinttypes>

#include "cetech/resource_compiler/compilatorapi.h"
#include "cetech/filesystem/filesystem.h"
#include "celib/string/stringid_types.h"
#include "rapidjson/document.h"

namespace cetech {
    namespace resource_compiler {
        typedef void (* resource_compiler_clb_t)(const char*,
                                                 CompilatorAPI&);

        void register_compiler(StringId64_t type,
                               resource_compiler_clb_t clb);
        void compile_all();
    }

    namespace resource_compiler_globals {
        void init();
        void shutdown();
    }
}
