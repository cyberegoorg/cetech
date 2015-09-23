#include "resource_manager.h"
#include "common/container/container_types.h"
#include "common/container/queue.inl.h"
#include "common/container/hash.inl.h"
#include "common/memory/memory.h"
#include "common/string/stringid.inl.h"

#include "cvars/cvars.h"

#include "runtime/runtime.h"

#include <new>
#include <cstdio>

namespace cetech {
    class ResourceManagerImplementation : public ResourceManager {
        friend class ResourceManager;

        Hash < void* > _data_map;
        Hash < resource_loader_clb_t > _load_clb_map;
        Hash < resource_unloader_clb_t > _unload_clb_map;
        Hash < resource_compiler_clb_t > _compile_clb_map;

        ResourceManagerImplementation(Allocator & allocator) : _data_map(allocator),
                                                               _load_clb_map(allocator),
                                                               _unload_clb_map(allocator),
                                                               _compile_clb_map(allocator) {}

        virtual void compile(const char* filename) {
            uint64_t type = 0;
            uint64_t name = 0;
            calc_hash(filename, type, name);

            log::info("resource_manager",
                      "Compile \"%s\" => (" "%" PRIx64 ", " "%" PRIx64 ").",
                      filename,
                      type,
                      name);

            char output_filename[512] = {0};
            make_resource_full_path(output_filename,
                                    cvars::rm_build_dir.value_str,
                                    type,
                                    name);

            char input_filename[512] = {0};
            make_full_path(input_filename, cvars::rm_source_dir.value_str, filename);

            File f_in, f_out;
            f_in = runtime::file::from_file(input_filename, "rb");
            if (runtime::file::is_null(f_in)) {
                log::error("resource_manager", "Could not open source file \"%s\"", input_filename);

                return;
            }

            f_out = runtime::file::from_file(output_filename, "wb");

            resource_compiler_clb_t clb = hash::get < resource_compiler_clb_t >
                                          (this->_compile_clb_map, type, nullptr);

            if (clb == nullptr) {
                log::error("resource_manager", "Resource type " "%" PRIx64 " not register compiler.", type);
                return;
            }

            clb(f_in, f_out);

            runtime::file::close(f_in);
            runtime::file::close(f_out);
        }

        virtual void load(StringId64_t type, const StringId64_t* names, const uint32_t count) {
            StringId64_t name = 0;
            for (uint32_t i = 0; i < count; ++i) {
                name = names[i];

                log::info("resource_manager", "Loading resource (" "%" PRIx64 ", " "%" PRIx64 ").", type, name);

                resource_loader_clb_t clb = hash::get < resource_loader_clb_t >
                                            (this->_load_clb_map, type, nullptr);

                if (clb == nullptr) {
                    log::error("resource_manager", "Resource type " "%" PRIx64 " not register loader.", type);
                    return;
                }

                char filename[512] = {0};
                make_resource_full_path(filename, cvars::rm_build_dir.value_str, type, name);

                File f = runtime::file::from_file(filename, "r");

                if (runtime::file::is_null(f)) {
                    log::error("resource_manager",
                               "Could not open resouce (" "%" PRIx64 ", " "%" PRIx64 ").",
                               type,
                               name);
                    return;
                }

                void* data = clb(f, memory_globals::default_allocator());

                if (data == nullptr) {
                    log::error("resource_manager",
                               "Could not load resouce (" "%" PRIx64 ", " "%" PRIx64 ").",
                               type,
                               name);
                    return;
                }

                hash::set(this->_data_map, type ^ name, data);
            }
        }

        virtual void unload(StringId64_t type, const StringId64_t* names, const uint32_t count) {
            StringId64_t name = 0;
            for (uint32_t i = 0; i < count; ++i) {
                name = names[i];

                resource_unloader_clb_t clb = hash::get < resource_unloader_clb_t >
                                              (this->_unload_clb_map, type, nullptr);

                if (clb == nullptr) {
                    log::error("resource_manager", "Resource type " "%" PRIx64 " not register unloader.", type);
                    return;
                }

                void* data = (void*) get(type, name);
                clb(memory_globals::default_allocator(), data);

                hash::remove(this->_data_map, type ^ name);
            }
        }

        virtual bool can_get(StringId64_t type, StringId64_t* names, const uint32_t count) {
            StringId64_t name = 0;
            for (uint32_t i = 0; i < count; ++i) {
                name = names[i];

                if (!hash::has(this->_data_map, type ^ name)) {
                    return false;
                }
            }

            return true;
        }

        virtual const void* get(StringId64_t type, StringId64_t name) {
            return hash::get < void* > (this->_data_map, type ^ name, nullptr);
        }

        virtual void register_compiler(StringId64_t type, resource_compiler_clb_t clb) {
            hash::set(this->_compile_clb_map, type, clb);
        }

        virtual void register_loader(StringId64_t type, resource_loader_clb_t clb) {
            hash::set(this->_load_clb_map, type, clb);
        }

        virtual void register_unloader(StringId64_t type, resource_unloader_clb_t clb) {
            hash::set(this->_unload_clb_map, type, clb);
        }


        CE_INLINE void calc_hash(const char* path, StringId64_t& type, StringId64_t& name) {
            const char* t = strrchr(path, '.');
            CE_CHECK_PTR(t);

            const uint32_t sz = t - path;
            t = t + 1;

            const uint32_t len = strlen(t);

            type = stringid64::from_cstring_len(t, len);
            name = stringid64::from_cstring_len(path, sz);
        }

        CE_INLINE void make_full_path(char* buffer, const char* base_path, const char* filename) {
            std::sprintf(buffer, "%s%s", base_path, filename);
        }

        CE_INLINE void resource_id_to_str(char* buffer, const StringId64_t& type, const StringId64_t& name) {
            std::sprintf(buffer, "%" PRIx64 "%" PRIx64, type, name);
        }

        CE_INLINE void make_resource_full_path(char* buffer,
                                               const char* base_path,
                                               const StringId64_t& type,
                                               const StringId64_t& name) {
            char resource_srt[32 + 1] = {0};
            resource_id_to_str(resource_srt, type, name);
            make_full_path(buffer, base_path, resource_srt);
        }
    };

    ResourceManager* ResourceManager::make(Allocator& alocator) {
        return MAKE_NEW(alocator, ResourceManagerImplementation, alocator);
    }

    void ResourceManager::destroy(Allocator& alocator, ResourceManager* rm) {
        MAKE_DELETE(memory_globals::default_allocator(), ResourceManager, rm);
    }
}