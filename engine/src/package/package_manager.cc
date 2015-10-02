#include "package_manager.h"
#include "resource/resource_manager.h"
#include "common/container/container_types.h"
#include "common/container/queue.inl.h"
#include "common/container/hash.inl.h"
#include "common/memory/memory.h"
#include "common/crypto/murmur_hash.inl.h"
#include "application.h"

#include "os/os.h"

#include "package/package_resource.h"

#include <new>
#include <cstdio>
namespace cetech {
    struct PackageLoaderTask {
        StringId64_t type;
        StringId64_t* names;
        uint32_t count;

        ResourceManager* rm;
    };

    enum {
        TASK_POOL_SIZE = 4096
    };

    static PackageLoaderTask loader_task_pool[TASK_POOL_SIZE];
    static uint32_t loader_task_pool_idx;

    CE_INLINE PackageLoaderTask& new_loader_task() {
        return loader_task_pool[(loader_task_pool_idx++) % TASK_POOL_SIZE];
    };

    class PackageManagerImplementation : public PackageManager {
        public:
            friend class PackageManager;

            static void package_loader_task(void* data) {
                log::debug("package_manager.loader.task", "Loading package");

                PackageLoaderTask* pkg_loader = (PackageLoaderTask*) data;

                Array < void* > loaded_data(memory_globals::default_allocator());
                array::reserve(loaded_data, pkg_loader->count);

                pkg_loader->rm->load(array::begin(loaded_data), pkg_loader->type, pkg_loader->names, pkg_loader->count);
                pkg_loader->rm->add_loaded(array::begin(
                                               loaded_data), pkg_loader->type, pkg_loader->names, pkg_loader->count);
            }

            virtual void load(StringId64_t name) final {
                const void* res = application_globals::device().resource_manager().get(resource_package::type_hash(), name);

                if (res == nullptr) {
                    log::error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                    return;
                }

                resource_package::Header* header = (resource_package::Header*)res;
                resource_package::TypeHeader* type_header = (resource_package::TypeHeader*)(header + 1);

                ResourceManager& rm = application_globals::device().resource_manager();
                TaskManager& tm = application_globals::device().task_manager();

                const uint64_t types_count = header->count;
                for (uint64_t i = 0; i < types_count; ++i) {
                    uint32_t count = type_header[i].count;
                    StringId64_t type = type_header[i].type;
                    StringId64_t* names = (StringId64_t*)(res + type_header[i].offset);

                    PackageLoaderTask& pkg_task = new_loader_task();
                    pkg_task.count = count;
                    pkg_task.names = names;
                    pkg_task.type = type;
                    pkg_task.rm = &rm;

                    TaskManager::TaskID tid = tm.add_begin(package_loader_task, &pkg_task, 0, NULL_TASK, NULL_TASK);
                    tm.add_end(&tid, 1);
                }
            }

            virtual void unload(StringId64_t name) final {
                const void* res = application_globals::device().resource_manager().get(resource_package::type_hash(), name);

                if (res == nullptr) {
                    log::error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                    return;
                }

                resource_package::Header* header = (resource_package::Header*)res;
                resource_package::TypeHeader* type_header = (resource_package::TypeHeader*)(header + 1);

                ResourceManager& rm = application_globals::device().resource_manager();
                const uint64_t types_count = header->count;
                for (uint64_t i = 0; i < types_count; ++i) {
                    uint32_t count = type_header[i].count;
                    StringId64_t type = type_header[i].type;
                    StringId64_t* names = (StringId64_t*)(res + type_header[i].offset);

                    rm.unload(type, names, count);
                }
            }

            virtual bool is_loaded(StringId64_t name) final {
                const void* res = application_globals::device().resource_manager().get(resource_package::type_hash(), name);

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

                    if (!application_globals::device().resource_manager().can_get(type, names, count)) {
                        return false;
                    }
                }

                return true;
            }

            virtual void flush(StringId64_t name) final {
                while (!is_loaded(name)) {
                    // TODO: Do some job for task manager
                }
            }
    };

    PackageManager* PackageManager::make(Allocator& alocator) {
        return MAKE_NEW(alocator, PackageManagerImplementation);
    }

    void PackageManager::destroy(Allocator& alocator, PackageManager* pm) {
        MAKE_DELETE(memory_globals::default_allocator(), PackageManager, pm);
    }
}