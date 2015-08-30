#include "common/resource_manager.h"
#include "container/container_types.h"
#include "container/queue.h"
#include "container/hash.h"
#include "memory/memory.h"
#include "runtime/runtime.h"

#include <new>
#include <cstdio>

namespace cetech {

    struct ResourceManager {
        Hash < void* > _data_map;

        Hash < resource_manager::resource_loader_clb_t > _load_clb_map;
        Hash < resource_manager::resource_compiler_clb_t > _compile_clb_map;

        ResourceManager() : _data_map(memory_globals::default_allocator()),
                            _load_clb_map(memory_globals::default_allocator()),
                            _compile_clb_map(memory_globals::default_allocator()) {}
    };

    namespace resource_manager_globals {
        static ResourceManager* rm = nullptr;

        void init() {
            rm = MAKE_NEW(memory_globals::default_allocator(), ResourceManager);
        }

        void shutdown() {
            MAKE_DELETE(memory_globals::default_allocator(), ResourceManager, rm);
            rm = nullptr;
        }
    }


    namespace resource_manager {
        CE_INLINE void calc_hash(const char* path, uint64_t& type, uint64_t& name) {
            const char* t = strrchr(path, '.');
            CE_CHECK_PTR(t);

            const uint32_t sz = t - path;

            t = t + 1;

            const uint32_t len = strlen(t);

            type = murmur_hash_64(t, len, 22);
            name = murmur_hash_64(path, sz, 22);


            printf("type: " "%" PRIx64 ", " "%" PRIx64 "\n", type, name);
        }

        void compile(const char* filename) {
            uint64_t type = 0;
            uint64_t name = 0;
            calc_hash(filename, type, name);


            char output_filename[512] = {0};
            std::sprintf(output_filename, "./data/build/" "%" PRIx64 "%" PRIx64, type, name);

            char input_filename[512] = {0};
            std::sprintf(input_filename, "./data/src/%s", filename);
            
            File f_in, f_out;

            f_in = runtime::file::from_file(input_filename, "rb");
            f_out = runtime::file::from_file(output_filename, "wb");

            resource_compiler_clb_t clb = hash::get < resource_compiler_clb_t >
                                          (resource_manager_globals::rm->_compile_clb_map, type, nullptr);

            if (clb == nullptr) {
                log::error("resource_manager", "Resource type " "%" PRIx64 " not register compiler.", type);
                return;
            }

            clb(f_in, f_out);

            runtime::file::close(f_in);
            runtime::file::close(f_out);
        }

        void load(uint64_t type, uint64_t name) {
            resource_loader_clb_t clb = hash::get < resource_loader_clb_t >
                                        (resource_manager_globals::rm->_load_clb_map, type, nullptr);

            if (clb == nullptr) {
                log::error("resource_manager", "Resource type " "%" PRIx64 " not register loader.", type);
                return;
            }

            char filename[512] = {0};
            std::sprintf(filename, "./data/build/" "%" PRIx64 "%" PRIx64, type, name);

            File f = runtime::file::from_file(filename, "r");

            void* data = clb(f, memory_globals::default_allocator());

            if (data == nullptr) {
                log::error("resource_manager", "Could not load resouce.");
                return;
            }

            hash::set(resource_manager_globals::rm->_data_map, type ^ name, data);
        }

        bool can_get(uint64_t type, uint64_t name) {
            return hash::has(resource_manager_globals::rm->_data_map, type ^ name);
        }

        const void* get(uint64_t type, uint64_t name) {
            return hash::get < void* > (resource_manager_globals::rm->_data_map, type ^ name, nullptr);
        }

        void register_compiler_clb(uint64_t type, resource_compiler_clb_t clb) {
            hash::set(resource_manager_globals::rm->_compile_clb_map, type, clb);
        }

        void register_loader_clb(uint64_t type, resource_loader_clb_t clb) {
            hash::set(resource_manager_globals::rm->_load_clb_map, type, clb);
        }
    }
}