#pragma once

#include <cinttypes>

#include "cetech/filesystem/filesystem.h"
#include "cetech/task_manager/task_manager.h"

#include "celib/string/stringid_types.h"
#include "celib/memory/memory_types.h"

#include "rapidjson/document.h"

namespace cetech {
    class ResourceCompiler {
        public:
            typedef void (* resource_compiler_clb_t)(FSFile*, FSFile*);

            virtual ~ResourceCompiler() {}

            virtual void register_compiler(StringId64_t type, resource_compiler_clb_t clb) = 0;
            virtual void compile_all_resource() = 0;

            static ResourceCompiler* make(Allocator& allocator, FileSystem* build_fs);
            static void destroy(Allocator& allocator, ResourceCompiler* rm);
    };
}