#pragma once

#include <cinttypes>

#include "cetech/filesystem/filesystem.h"
#include "cetech/task_manager/task_manager.h"

#include "celib/string/stringid_types.h"
#include "celib/memory/memory_types.h"

#include "rapidjson/document.h"

namespace cetech {
    struct CompilatorAPI {
        CompilatorAPI(FileSystem * src_fs, FileSystem * build_fs, FSFile * resource_file, FSFile * build_file);
        ~CompilatorAPI();

        bool resource_to_json(rapidjson::Document& document);
        bool add_dependency(const char* path);

        size_t resource_file_size();
        bool read_resource_file(char* buffer);

        void write_to_build(const void* buffer, size_t size);

        struct Implementation;
        Implementation* _impl;
    };

    class ResourceCompiler {
        public:
            typedef void (* resource_compiler_clb_t)(const char*, FSFile*, FSFile*, CompilatorAPI&);

            virtual ~ResourceCompiler() {}

            virtual void register_compiler(StringId64_t type, resource_compiler_clb_t clb) = 0;
            virtual void compile_all_resource() = 0;

            static ResourceCompiler* make(Allocator& allocator, FileSystem* build_fs);
            static void destroy(Allocator& allocator, ResourceCompiler* rm);
    };
}