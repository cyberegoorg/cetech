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
            resource_package::TypeHeader* type_header = (resource_package::TypeHeader*)(header + 1);
            
            ResourceManager& rm = device_globals::device().resource_manager();
            const uint64_t types_count = header->count;
            for (uint64_t i = 0; i < types_count; ++i) {
                uint32_t count = type_header[i].count;
                StringId64_t type = type_header[i].type;
                StringId64_t* names = (StringId64_t*)(res + type_header[i].offset);

                rm.load(type, names, count);
            }
        }

        virtual void unload(StringId64_t name) {
            const void* res = device_globals::device().resource_manager().get(resource_package::type_hash(), name);

            if (res == nullptr) {
                log::error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                return;
            }

            resource_package::Header* header = (resource_package::Header*)res;
            resource_package::TypeHeader* type_header = (resource_package::TypeHeader*)(header + 1);

            ResourceManager& rm = device_globals::device().resource_manager();
            const uint64_t types_count = header->count;
            for (uint64_t i = 0; i < types_count; ++i) {
                uint32_t count = type_header[i].count;
                StringId64_t type = type_header[i].type;
                StringId64_t* names = (StringId64_t*)(res + type_header[i].offset);

                rm.unload(type, names, count);
            }
        }

        virtual bool is_loaded(StringId64_t name) {
            const void* res = device_globals::device().resource_manager().get(resource_package::type_hash(), name);

            if (res == nullptr) {
                log::error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                return false;
            }


            resource_package::Header* header = (resource_package::Header*)res;
            resource_package::TypeHeader* type_header = (resource_package::TypeHeader*)(header + 1);
            
            const uint64_t types_count = header->count;
            for (uint64_t i = 0; i < types_count; ++i) {
                uint32_t count = type_header[i].count;
                StringId64_t type = type_header[i].type;
                StringId64_t* names = (StringId64_t*)(res + type_header[i].offset);

                if (!device_globals::device().resource_manager().can_get(type, names, count)) {
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