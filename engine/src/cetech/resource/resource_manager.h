#pragma once

#include <cinttypes>

#include "cetech/filesystem/filesystem.h"
#include "cetech/task_manager/task_manager.h"

#include "celib/string/stringid_types.h"
#include "celib/memory/memory_types.h"

#include "rapidjson/document.h"

namespace cetech {
    class ResourceManager {
        public:
            typedef void* (* resource_loader_clb_t)(FSFile*, Allocator&);
            typedef void (* resource_unloader_clb_t)(Allocator&, void*);

            virtual ~ResourceManager() {}

            virtual void register_loader(StringId64_t type, resource_loader_clb_t clb) = 0;
            virtual void register_unloader(StringId64_t type, resource_unloader_clb_t clb) = 0;

            virtual void load(void** loaded_data, StringId64_t type, const StringId64_t* names,
                              const uint32_t count) = 0;
            virtual void add_loaded(void** loaded_data,
                                    StringId64_t type,
                                    const StringId64_t* names,
                                    const uint32_t count) = 0;

            virtual void unload(StringId64_t type, const StringId64_t* names, const uint32_t count) = 0;

            virtual bool can_get(StringId64_t type, StringId64_t* names, const uint32_t count) = 0;
            virtual const void* get(StringId64_t type, StringId64_t name) = 0;

            static ResourceManager* make(Allocator& allocator, FileSystem* fs);
            static void destroy(Allocator& allocator, ResourceManager* rm);
    };
}