#include <cstdio>

#include "resource_manager.h"
#include "cetech/application/application.h"

#include "celib/container/container_types.h"
#include "celib/container/queue.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/memory/memory.h"
#include "celib/string/stringid.inl.h"

#include "cetech/cvars/cvars.h"
#include "celib/platform/dir.h"
#include "celib/platform/thread.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/memorybuffer.h"

namespace cetech {
    class ResourceManagerImplementation final : public ResourceManager {
        public:
            friend class ResourceManager;

            FileSystem* _fs;

            Hash < void* > _data_map;
            Hash < uint32_t > _data_refcount_map;

            Hash < resource_loader_clb_t > _load_clb_map;
            Hash < resource_unloader_clb_t > _unload_clb_map;

            Spinlock add_lock;
            bool autoreload;

            ResourceManagerImplementation(FileSystem * fs, Allocator & allocator) : _fs(fs), _data_map(allocator),
                                                                                    _data_refcount_map(allocator),
                                                                                    _load_clb_map(allocator),
                                                                                    _unload_clb_map(allocator),
                                                                                    autoreload(true) {}

            virtual void load(void** loaded_data, StringId64_t type, const StringId64_t* names,
                              const uint32_t count) final {
                StringId64_t name = 0;

                resource_loader_clb_t clb = hash::get < resource_loader_clb_t >
                                            (this->_load_clb_map, type, nullptr);

                if (clb == nullptr) {
                    log::error("resource_manager", "Resource type " "%" PRIx64 " not register loader.", type);
                    return;
                }

                for (uint32_t i = 0; i < count; ++i) {
                    name = names[i];

                    log::info("resource_manager", "Loading resource (" "%" PRIx64 ", " "%" PRIx64 ").", type, name);

                    char resource_srt[32 + 1] = {0};
                    resource_id_to_str(resource_srt, type, name);

                    FSFile* f = _fs->open(resource_srt, FSFile::READ);

                    if (!f->is_valid()) {
                        log::error("resource_manager",
                                   "Could not open resouce (" "%" PRIx64 ", " "%" PRIx64 ").",
                                   type,
                                   name);
                        loaded_data[i] = nullptr;
                        continue;
                    }

                    void* data = clb(f, memory_globals::default_allocator());

                    if (data == nullptr) {
                        log::error("resource_manager",
                                   "Could not load resouce (" "%" PRIx64 ", " "%" PRIx64 ").",
                                   type,
                                   name);
                    }

                    loaded_data[i] = data;
close:
                    _fs->close(f);
                }
            }

            virtual void add_loaded(void** loaded_data,
                                    StringId64_t type,
                                    const StringId64_t* names,
                                    const uint32_t count) final {
                thread::spin_lock(add_lock);
                for (uint32_t i = 0; i < count; ++i) {
                    const StringId64_t name = names[i];

                    hash::set(this->_data_map, type ^ name, loaded_data[i]);
                    inc_reference(type, name);
                }

                thread::spin_unlock(add_lock);
            };


            virtual void unload(StringId64_t type, const StringId64_t* names, const uint32_t count) final {
                resource_unloader_clb_t clb = hash::get < resource_unloader_clb_t >
                                              (this->_unload_clb_map, type, nullptr);

                if (clb == nullptr) {
                    log::error("resource_manager", "Resource type " "%" PRIx64 " not register unloader.", type);
                    return;
                }

                StringId64_t name = 0;
                for (uint32_t i = 0; i < count; ++i) {
                    name = names[i];

                    if (!dec_reference(type, name)) {
                        continue;
                    }

                    void* data = (void*) get(type, name);
                    clb(memory_globals::default_allocator(), data);

                    hash::remove(this->_data_map, type ^ name);
                }
            }

            virtual bool can_get(StringId64_t type, StringId64_t* names, const uint32_t count) final {
                StringId64_t name = 0;
                for (uint32_t i = 0; i < count; ++i) {
                    name = names[i];

                    if (!hash::has(this->_data_map, type ^ name)) {
                        thread::spin_unlock(add_lock);
                        return false;
                    }
                }

                return true;
            }

            void load_now(StringId64_t type, StringId64_t* names, const uint32_t count) {
                Array < void* > loaded_data(memory_globals::default_allocator());
                array::reserve(loaded_data, count);

                load(array::begin(loaded_data), type, names, count);
                add_loaded(array::begin(loaded_data), type, names, count);
            }

            virtual const void* get(StringId64_t type, StringId64_t name) final {
                if (autoreload) {
                    if (!can_get(type, &name, 1)) {
                        load_now(type, &name, 1);
                    }
                }

                return hash::get < void* > (this->_data_map, type ^ name, nullptr);
            }

            virtual void register_loader(StringId64_t type, resource_loader_clb_t clb) final {
                hash::set(this->_load_clb_map, type, clb);
            }

            virtual void register_unloader(StringId64_t type, resource_unloader_clb_t clb) final {
                hash::set(this->_unload_clb_map, type, clb);
            }

            CE_INLINE void inc_reference(StringId64_t type, const StringId64_t name) {
                const uint32_t counter = hash::get < uint32_t > (_data_refcount_map, type ^ name, 0) + 1;

                log::debug("resource_manager",
                           "Inc reference for (%" PRIx64 ", %" PRIx64 ") counter == %d ",
                           type,
                           name,
                           counter);

                hash::set(_data_refcount_map, type ^ name, counter);
            }

            CE_INLINE bool dec_reference(StringId64_t type, const StringId64_t name) {
                const uint32_t counter = hash::get < uint32_t > (_data_refcount_map, type ^ name, 1) - 1;

                log::debug("resource_manager",
                           "Dec reference for  (%" PRIx64 ", %" PRIx64 ") counter == %d ",
                           type,
                           name,
                           counter);

                hash::set(_data_refcount_map, type ^ name, counter);

                return counter == 0;
            }

            static CE_INLINE void calc_hash(const char* path, StringId64_t& type, StringId64_t& name) {
                const char* t = strrchr(path, '.');
                CE_CHECK_PTR(t);

                const uint32_t sz = t - path;
                t = t + 1;

                const uint32_t len = strlen(t);

                type = stringid64::from_cstringn(t, len);
                name = stringid64::from_cstringn(path, sz);
            }

            static CE_INLINE void resource_id_to_str(char* buffer, const StringId64_t& type, const StringId64_t& name) {
                std::sprintf(buffer, "%" PRIx64 "%" PRIx64, type, name);
            }
    };

    ResourceManager* ResourceManager::make(Allocator& allocator, FileSystem* fs) {
        return MAKE_NEW(allocator, ResourceManagerImplementation, fs, allocator);
    }

    void ResourceManager::destroy(Allocator& allocator, ResourceManager* rm) {
        MAKE_DELETE(memory_globals::default_allocator(), ResourceManager, rm);
    }
}