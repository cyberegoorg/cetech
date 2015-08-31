#include "common/package_manager.h"
#include "common/resource_manager.h"

#include "container/container_types.h"
#include "container/queue.h"
#include "container/hash.h"
#include "memory/memory.h"
#include "runtime/runtime.h"
#include "common/murmur_hash.h"

#include "resources/package.h"

#include <new>
#include <cstdio>

namespace cetech {

    struct PackageManager {
        uint64_t type_hash;
        
        PackageManager() {
            type_hash = murmur_hash_64("package", strlen("package"), 22);
        }
    };

    namespace package_manager_globals {
        static PackageManager* pm = nullptr;

        void init() {
            pm = MAKE_NEW(memory_globals::default_allocator(), PackageManager);
        }

        void shutdown() {
            MAKE_DELETE(memory_globals::default_allocator(), PackageManager, pm);
            pm = nullptr;
        }
    }


    namespace package_manager {
        void load(uint64_t name) {
            const void* res = resource_manager::get(package_manager_globals::pm->type_hash, name);
            
            resource_package::Header *header = (resource_package::Header*)res;
            resource_package::Item *items =(resource_package::Item*)( res + sizeof(resource_package::Header));
            
            const uint64_t count = header->count;
            for(uint64_t i = 0; i < count; ++i) {
                resource_package::Item &item = items[i];
                resource_manager::load(item.type, item.name);
            }
        }
        
        void unload(uint64_t name) {
            const void* res = resource_manager::get(package_manager_globals::pm->type_hash, name);
            
            resource_package::Header *header = (resource_package::Header*)res;
            resource_package::Item *items =(resource_package::Item*)( res + sizeof(resource_package::Header));
            
            const uint64_t count = header->count;
            for(uint64_t i = 0; i < count; ++i) {
                resource_package::Item &item = items[i];
                resource_manager::unload(item.type, item.name);
            }
        }
    }
}