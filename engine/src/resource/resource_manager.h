#pragma once

#include <cinttypes>

#include "filesystem/filesystem.h"
#include "task_manager.h"

#include "common/string/stringid_types.h"
#include "common/memory/memory_types.h"
#include "os/os_types.h"

namespace cetech {
    class ResourceManager {
        public:
            typedef void (* resource_compiler_clb_t)(File*, File*);
            typedef void* (* resource_loader_clb_t)(File*, Allocator&);
            typedef void (* resource_unloader_clb_t)(Allocator&, void*);

            virtual ~ResourceManager() {}

            virtual void register_compiler(StringId64_t type, resource_compiler_clb_t clb) = 0;
            virtual void register_loader(StringId64_t type, resource_loader_clb_t clb) = 0;
            virtual void register_unloader(StringId64_t type, resource_unloader_clb_t clb) = 0;

            virtual TaskManager::TaskID compile(FileSystem* source_fs) = 0;

            virtual void load(StringId64_t type, const StringId64_t* names, const uint32_t count) = 0;
            virtual void unload(StringId64_t type, const StringId64_t* names, const uint32_t count) = 0;

            virtual bool can_get(StringId64_t type, StringId64_t* names, const uint32_t count) = 0;
            virtual const void* get(StringId64_t type, StringId64_t name) = 0;

            static ResourceManager* make(Allocator& alocator, FileSystem* fs);
            static void destroy(Allocator& alocator, ResourceManager* rm);
    };
}