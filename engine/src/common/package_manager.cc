#include "common/package_manager.h"
#include "common/resource_manager.h"

#include "container/container_types.h"
#include "container/queue.h"
#include "container/hash.h"
#include "memory/memory.h"
#include "runtime/runtime.h"
#include "common/murmur_hash.h"

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
        struct PackageResourceHeader {
            uint64_t count;
        };
        
        struct PackageResourceItem {
            uint64_t type;
            uint64_t name;
        };
        
        void load(uint64_t name) {
            const void* res = resource_manager::get(package_manager_globals::pm->type_hash, name);
            
            PackageResourceHeader *header = (PackageResourceHeader*)res;
            PackageResourceItem *items =(PackageResourceItem*)( res + sizeof(PackageResourceHeader));
            
            const uint64_t count = header->count;
            for(uint64_t i = 0; i < count; ++i) {
                PackageResourceItem &item = items[i];
                resource_manager::load(item.type, item.name);
            }
        }
    }
}