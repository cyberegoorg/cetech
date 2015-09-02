#include "common/resource/package_manager.h"
#include "common/resource/resource_manager.h"
#include "common/container/container_types.h"
#include "common/container/queue.h"
#include "common/container/hash.h"
#include "common/memory/memory.h"
#include "common/murmur_hash.h"

#include "runtime/runtime.h"

#include "resources/package.h"

#include <new>
#include <cstdio>

namespace cetech {

    struct PackageManager {
        StringId64_t type_hash;

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
        StringId64_t type_name() {
            return package_manager_globals::pm->type_hash;
        }

        void load(StringId64_t name) {
            const void* res = resource_manager::get(package_manager_globals::pm->type_hash, name);

            if (res == nullptr) {
                log::error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                return;
            }

            resource_package::Header* header = (resource_package::Header*)res;
            resource_package::Item* items = (resource_package::Item*)(res + sizeof(resource_package::Header));

            const uint64_t count = header->count;
            for (uint64_t i = 0; i < count; ++i) {
                resource_package::Item& item = items[i];
                resource_manager::load(item.type, item.name);
            }
        }

        void unload(StringId64_t name) {
            const void* res = resource_manager::get(package_manager_globals::pm->type_hash, name);

            if (res == nullptr) {
                log::error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                return;
            }

            resource_package::Header* header = (resource_package::Header*)res;
            resource_package::Item* items = (resource_package::Item*)(res + sizeof(resource_package::Header));

            const uint64_t count = header->count;
            for (uint64_t i = 0; i < count; ++i) {
                resource_package::Item& item = items[i];
                resource_manager::unload(item.type, item.name);
            }
        }

        bool is_loaded(StringId64_t name) {
            const void* res = resource_manager::get(package_manager_globals::pm->type_hash, name);

            if (res == nullptr) {
                log::error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                return false;
            }

            resource_package::Header* header = (resource_package::Header*)res;
            resource_package::Item* items = (resource_package::Item*)(res + sizeof(resource_package::Header));

            const uint64_t count = header->count;
            for (uint64_t i = 0; i < count; ++i) {
                resource_package::Item& item = items[i];

                if (!resource_manager::can_get(item.type, item.name)) {
                    return false;
                }
            }

            return true;
        }

        void flush(StringId64_t name) {
            while (!is_loaded(name)) {}
        }
    }
}