#include "cetech/resource_manager/resource_manager.h"
#include "celib/container/container_types.h"
#include "celib/container/queue.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/memory/memory.h"
#include "celib/crypto/murmur_hash.inl.h"
#include "cetech/application/application.h"

#include "cetech/os/os.h"

#include "cetech/package_manager/package_manager.h"
#include "cetech/package_manager/package_resource.h"

#include "cetech/cvars/cvars.h"
#include "celib/string/stringid.inl.h"

#include <new>
#include <cstdio>
namespace cetech {
    struct PackageLoaderTask {
        StringId64_t type;
        StringId64_t* names;
        uint32_t count;
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
                log_globals::log().debug("package_manager.loader.task", "Loading package");

                PackageLoaderTask* pkg_loader = (PackageLoaderTask*) data;

                Array < char* > loaded_data(memory_globals::default_allocator());
                array::reserve(loaded_data, pkg_loader->count);

                resource_manager::load(array::begin(loaded_data), pkg_loader->type, pkg_loader->names,
                                       pkg_loader->count);
                resource_manager::add_loaded(array::begin(
                                                 loaded_data), pkg_loader->type, pkg_loader->names, pkg_loader->count);
            }

            virtual void load(StringId64_t name) final {
                const char* res = resource_manager::get(resource_package::type_hash(), name);

                if (res == nullptr) {
                    log_globals::log().error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                    return;
                }

                resource_package::Header* header = (resource_package::Header*)res;
                resource_package::TypeHeader* type_header = (resource_package::TypeHeader*)(header + 1);

                TaskManager& tm = application_globals::app().task_manager();

                const uint64_t types_count = header->count;
                for (uint64_t i = 0; i < types_count; ++i) {
                    uint32_t count = type_header[i].count;
                    StringId64_t type = type_header[i].type;
                    StringId64_t* names = (StringId64_t*)(res + type_header[i].offset);

                    PackageLoaderTask& pkg_task = new_loader_task();
                    pkg_task.count = count;
                    pkg_task.names = names;
                    pkg_task.type = type;

                    TaskManager::TaskID tid = tm.add_begin(package_loader_task, &pkg_task, 0, NULL_TASK, NULL_TASK);
                    tm.add_end(&tid, 1);
                }
            }

            virtual void unload(StringId64_t name) final {
                const char* res = resource_manager::get(resource_package::type_hash(), name);

                if (res == nullptr) {
                    log_globals::log().error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                    return;
                }

                resource_package::Header* header = (resource_package::Header*)res;
                resource_package::TypeHeader* type_header = (resource_package::TypeHeader*)(header + 1);

                const uint64_t types_count = header->count;
                for (uint64_t i = 0; i < types_count; ++i) {
                    uint32_t count = type_header[i].count;
                    StringId64_t type = type_header[i].type;
                    StringId64_t* names = (StringId64_t*)(res + type_header[i].offset);

                    resource_manager::unload(type, names, count);
                }
            }

            virtual bool is_loaded(StringId64_t name) final {
                const char* res =
                    resource_manager::get(resource_package::type_hash(), name);

                if (res == nullptr) {
                    //log_globals::log().error("package_manager", "Could not get resource for package " "%" PRIx64, name);
                    return false;
                }

                resource_package::Header* header = (resource_package::Header*)res;
                resource_package::TypeHeader* type_header = (resource_package::TypeHeader*)(header + 1);

                const uint64_t types_count = header->count;
                for (uint64_t i = 0; i < types_count; ++i) {
                    uint32_t count = type_header[i].count;
                    StringId64_t type = type_header[i].type;
                    StringId64_t* names = (StringId64_t*)(res + type_header[i].offset);

                    if (!resource_manager::can_get(type, names, count)) {
                        return false;
                    }
                }

                return true;
            }

            virtual void flush(StringId64_t name) final {
                TaskManager& tm = application_globals::app().task_manager();
                while (!is_loaded(name)) {
                    tm.do_work();
                }
            }

            virtual void load_boot_package() final {
                StringId64_t boot_pkg_name_h = stringid64::from_cstringn(cvars::boot_pkg.value_str,
                                                                         cvars::boot_pkg.str_len);

                // Load boot package
                char* package_data[1];
                resource_manager::load(package_data, resource_package::type_hash(), &boot_pkg_name_h, 1);
                resource_manager::add_loaded(package_data, resource_package::type_hash(), &boot_pkg_name_h, 1);

                load(boot_pkg_name_h);
                flush(boot_pkg_name_h);

                char* res = package_data[0];
                resource_package::Header* header = (resource_package::Header*)res;
                resource_package::TypeHeader* type_header = (resource_package::TypeHeader*)(header + 1);

                const uint64_t types_count = header->count;
                for (uint64_t i = 0; i < types_count; ++i) {
                    StringId64_t type = type_header[i].type;

                    if (type != resource_package::type_hash()) {
                        continue;
                    }

                    uint32_t count = type_header[i].count;
                    StringId64_t* names = (StringId64_t*)(res + type_header[i].offset);

                    for (uint32_t j = 0; j < count; ++j) {
                        load(names[j]);
                        flush(names[j]);
                    }

                    break;
                }
            }
    };

    PackageManager* PackageManager::make(Allocator& allocator) {
        return MAKE_NEW(allocator, PackageManagerImplementation);
    }

    void PackageManager::destroy(Allocator& allocator, PackageManager* pm) {
        MAKE_DELETE(allocator, PackageManager, pm);
    }
}