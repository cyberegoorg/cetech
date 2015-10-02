#include <cstdio>

#include "resource_manager.h"
#include "device.h"

#include "common/container/container_types.h"
#include "common/container/queue.inl.h"
#include "common/container/hash.inl.h"
#include "common/memory/memory.h"
#include "common/string/stringid.inl.h"

#include "cvars/cvars.h"
#include "os/os.h"

namespace cetech {
    class ResourceManagerImplementation final : public ResourceManager {
        public:
            friend class ResourceManager;

            FileSystem* _fs;

            Hash < void* > _data_map;
            Hash < uint32_t > _data_refcount_map;

            Hash < resource_loader_clb_t > _load_clb_map;
            Hash < resource_unloader_clb_t > _unload_clb_map;
            Hash < resource_compiler_clb_t > _compile_clb_map;

            os::Spinlock add_lock;

            ResourceManagerImplementation(FileSystem * fs, Allocator & allocator) : _fs(fs), _data_map(allocator),
                                                                                    _data_refcount_map(allocator),
                                                                                    _load_clb_map(allocator),
                                                                                    _unload_clb_map(allocator),
                                                                                    _compile_clb_map(allocator) {}


            struct CompileTask {
                FileSystem* source_fs;
                FileSystem* out_fs;

                char* filename;
                uint64_t name, type;

                resource_compiler_clb_t clb;
            };

            static void compile_task(void* data) {
                CompileTask* ct = (CompileTask*)data;

                log::info("resource_manager",
                          "Compile \"%s\" => (" "%" PRIx64 ", " "%" PRIx64 ").",
                          ct->filename,
                          ct->type,
                          ct->name);

                char output_filename[512] = {0};
                resource_id_to_str(output_filename, ct->type, ct->name);

                File* f_in;
                f_in = ct->source_fs->open(ct->filename, File::READ);
                if (!f_in->is_valid()) {
                    log::error("resource_manager", "Could not open source file \"%s\"", ct->filename);
                    return;
                }

                File* f_out = ct->out_fs->open(output_filename, File::WRITE);

                ct->clb(f_in, f_out);

                ct->source_fs->close(f_in);
                ct->out_fs->close(f_out);

                //MAKE_DELETE(memory_globals::default_allocator(), CompileTask, data);
            }

            virtual TaskManager::TaskID compile(FileSystem* source_fs) final {
                Array < char* > files(memory_globals::default_allocator());
                source_fs->list_directory(cvars::rm_source_dir.value_str, files);


                TaskManager& tm = device_globals::device().task_manager();
                TaskManager::TaskID top_compile_task = tm.add_empty_begin(0);

                const uint32_t files_count = array::size(files);
                for (uint32_t i = 0; i < files_count; ++i) {
                    const char* filename = files[i] + cvars::rm_source_dir.str_len; /* Base path */

                    uint64_t name, type = 0;
                    calc_hash(filename, type, name);

                    resource_compiler_clb_t clb = hash::get < resource_compiler_clb_t >
                                                  (this->_compile_clb_map, type, nullptr);

                    if (clb == nullptr) {
                        log::error("resource_manager", "Resource type " "%" PRIx64 " not register compiler.", type);
                        continue;
                    }

                    // TODO: Compile Task Pool, reduce alloc free, ringbuffer?
                    CompileTask* ct = MAKE_NEW(memory_globals::default_allocator(), CompileTask);
                    ct->source_fs = source_fs;
                    ct->out_fs = _fs;
                    ct->filename = strdup(filename);
                    ct->name = name;
                    ct->type = type;
                    ct->clb = clb;

                    TaskManager::TaskID tid = tm.add_begin(compile_task, ct, 0, NULL_TASK, top_compile_task);
                    tm.add_end(&tid, 1);
                }

                os::dir::listdir_free(files);

                tm.add_end(&top_compile_task, 1);
                return top_compile_task;
            }

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

                    File* f = _fs->open(resource_srt, File::READ);

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
                os::thread::spin_lock(add_lock);
                for (uint32_t i = 0; i < count; ++i) {
                    const StringId64_t name = names[i];

                    hash::set(this->_data_map, type ^ name, loaded_data[i]);
                    inc_reference(type, name);
                }

                os::thread::spin_unlock(add_lock);
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
                        os::thread::spin_unlock(add_lock);
                        return false;
                    }
                }

                return true;
            }

            virtual const void* get(StringId64_t type, StringId64_t name) final {
                return hash::get < void* > (this->_data_map, type ^ name, nullptr);
            }

            virtual void register_compiler(StringId64_t type, resource_compiler_clb_t clb) final {
                hash::set(this->_compile_clb_map, type, clb);
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

                type = stringid64::from_cstring_len(t, len);
                name = stringid64::from_cstring_len(path, sz);
            }

            static CE_INLINE void resource_id_to_str(char* buffer, const StringId64_t& type, const StringId64_t& name) {
                std::sprintf(buffer, "%" PRIx64 "%" PRIx64, type, name);
            }
    };

    ResourceManager* ResourceManager::make(Allocator& alocator, FileSystem* fs) {
        return MAKE_NEW(alocator, ResourceManagerImplementation, fs, alocator);
    }

    void ResourceManager::destroy(Allocator& alocator, ResourceManager* rm) {
        MAKE_DELETE(memory_globals::default_allocator(), ResourceManager, rm);
    }
}