#pragma once

#include <cinttypes>

#include "cetech/filesystem/filesystem.h"
#include "cetech/task_manager/task_manager.h"

#include "celib/string/stringid_types.h"
#include "celib/memory/memory_types.h"

#include "rapidjson/document.h"

namespace cetech {
    struct BuildDB;
    struct Compilator {

        Compilator(FileSystem * src_fs, FileSystem * build_fs, FSFile * resource_file) : src_fs(src_fs), build_fs(
                                                                                             build_fs), resource_file(
                                                                                             resource_file) {}

        bool resource_to_json(rapidjson::Document& document);
        
        bool add_dependency(const char* path);

        FileSystem* src_fs;
        FileSystem* build_fs;
        FSFile* resource_file;
        const char* filename;
        rapidjson::Document *dependency_index;
        BuildDB *bdb;
    };

    class ResourceCompiler {
        public:
            typedef void (* resource_compiler_clb_t)(const char*, FSFile*, FSFile*, Compilator&);

            virtual ~ResourceCompiler() {}

            virtual void register_compiler(StringId64_t type, resource_compiler_clb_t clb) = 0;
            virtual void compile_all_resource() = 0;

            static ResourceCompiler* make(Allocator& allocator, FileSystem* build_fs);
            static void destroy(Allocator& allocator, ResourceCompiler* rm);
    };
}