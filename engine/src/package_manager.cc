#include "package_manager.h"
#include "resource_manager.h"
#include "common/container/container_types.h"
#include "common/container/queue.inl.h"
#include "common/container/hash.inl.h"
#include "common/memory/memory.h"
#include "common/crypto/murmur_hash.h"
#include "device.h"

#include "runtime/runtime.h"

#include "resources/package.h"

#include <new>
#include <cstdio>

namespace cetech {
    class PackageManagerImplementation : public PackageManager {
        friend class PackageManager;

        virtual void load(StringId64_t name) {
            const void* res = device_globals::device().resource_manager().get(resource_package::type_hash(), name);

            if (res == nullptr) {
                log::error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                return;
            }

            resource_package::Header* header = (resource_package::Header*)res;
            resource_package::Item* items = (resource_package::Item*)(header + 1);

            const uint64_t count = header->count;
            for (uint64_t i = 0; i < count; ++i) {
                resource_package::Item& item = items[i];
                device_globals::device().resource_manager().load(item.type, item.name);
            }
        }

        virtual void unload(StringId64_t name) {
            const void* res = device_globals::device().resource_manager().get(resource_package::type_hash(), name);

            if (res == nullptr) {
                log::error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                return;
            }

            resource_package::Header* header = (resource_package::Header*)res;
            resource_package::Item* items = (resource_package::Item*)(header + 1);

            const uint64_t count = header->count;
            for (uint64_t i = 0; i < count; ++i) {
                resource_package::Item& item = items[i];
                device_globals::device().resource_manager().unload(item.type, item.name);
            }
        }

        virtual bool is_loaded(StringId64_t name) {
            const void* res = device_globals::device().resource_manager().get(resource_package::type_hash(), name);

            if (res == nullptr) {
                log::error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                return false;
            }

            resource_package::Header* header = (resource_package::Header*)res;
            resource_package::Item* items = (resource_package::Item*)(header + 1);

            const uint64_t count = header->count;
            for (uint64_t i = 0; i < count; ++i) {
                resource_package::Item& item = items[i];

                if (!device_globals::device().resource_manager().can_get(item.type, item.name)) {
                    return false;
                }
            }

            return true;
        }

        virtual void flush(StringId64_t name) {
            while (!is_loaded(name)) {}
        }
    };

    PackageManager* PackageManager::make(Allocator& alocator) {
        return MAKE_NEW(alocator, PackageManagerImplementation);
    }

    void PackageManager::destroy(Allocator& alocator, PackageManager* pm) {
        MAKE_DELETE(memory_globals::default_allocator(), PackageManager, pm);
    }
}