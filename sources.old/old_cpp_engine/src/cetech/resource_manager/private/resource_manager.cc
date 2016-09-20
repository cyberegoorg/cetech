#include <cstdio>

#include "cetech/resource_manager/resource_manager.h"
#include "cetech/application/application.h"

#include "celib/container/types.h"
#include "celib/container/queue.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/memory/memory.h"
#include "celib/string/stringid.inl.h"

#include "cetech/cvars/cvars.h"
#include "celib/thread/thread.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/memorybuffer.h"

namespace cetech {
    namespace {
        using namespace resource_manager;


        enum {
            MAX_TYPES = 64,
        };

        struct ResouceManagerData {
            Hash < uint32_t > _types_map;
            Hash < char* > *_data_map;
            Hash < uint32_t > *_data_refcount_map;

            Hash < resource_loader_clb_t > _load_clb_map;
            Hash < resource_unloader_clb_t > _unload_clb_map;
            Hash < resource_online_clb_t > _online_clb_map;
            Hash < resource_offline_clb_t > _offline_clb_map;

            Allocator& _allocator;

            Spinlock add_lock;
            uint32_t type_count;
            bool autoreload;

            explicit ResouceManagerData(Allocator& allocator) : _types_map(allocator),
                                                                _load_clb_map(allocator),
                                                                _unload_clb_map(allocator),
                                                                _online_clb_map(allocator),
                                                                _offline_clb_map(allocator),
                                                                _allocator(allocator),
                                                                type_count(0),
                                                                autoreload(true) {

                _data_map = memory::alloc_array < Hash < char* >, Allocator & > (allocator, MAX_TYPES, allocator);
                _data_refcount_map = memory::alloc_array < Hash < uint32_t >,
                Allocator & > (allocator, MAX_TYPES, allocator);
            }

            ~ResouceManagerData() {
                memory::dealloc_array(_allocator, _data_map, MAX_TYPES);
                memory::dealloc_array(_allocator, _data_refcount_map, MAX_TYPES);
            }
        };

        struct Globals {
            static const int MEMORY = sizeof(ResouceManagerData);
            char buffer[MEMORY];

            ResouceManagerData* data;

            Globals() : buffer {
                0
            }, data(0) {}
        } _globals;

        CE_INLINE void resource_id_to_str(char* buffer,
                                          const StringId64_t& type,
                                          const StringId64_t& name) {
            std::sprintf(buffer, "%" PRIx64 "%" PRIx64, type, name);
        }

    }

    namespace resource_manager {
        CE_INLINE void inc_reference(const uint32_t type_idx,
                                     const StringId64_t name) {
            auto& ref_count_map = _globals.data->_data_refcount_map[type_idx];
            const uint32_t counter = hash::get < uint32_t > (ref_count_map, name, 0) + 1;

            hash::set(ref_count_map, name, counter);
        }

        CE_INLINE bool dec_reference(const uint32_t type_idx,
                                     const StringId64_t name) {

            auto& ref_count_map = _globals.data->_data_refcount_map[type_idx];
            const uint32_t counter = hash::get < uint32_t > (ref_count_map, name, 0) - 1;

            hash::set(ref_count_map, name, counter);

            return counter == 0;
        }

        void load(char** loaded_data,
                  const StringId64_t type,
                  const StringId64_t* names,
                  const uint32_t count) {

            resource_loader_clb_t clb = hash::get < resource_loader_clb_t >
                                        (_globals.data->_load_clb_map, type, nullptr);

            for (uint32_t i = 0; i < count; ++i) {
                StringId64_t name = name = names[i];

                log::debug("resource",
                           "Loading resource " "%" PRIx64 "%" PRIx64 "",
                           type, name);

                char resource_srt[32 + 1] = {0};
                resource_id_to_str(resource_srt, type, name);

                FSFile& f = filesystem::open(BUILD_DIR, resource_srt, FSFile::READ);

                if (!f.is_valid()) {
                    log::error("resource",
                               "Could not open resouce (" "%" PRIx64 ", " "%" PRIx64 ").",
                               type,
                               name);
                    loaded_data[i] = nullptr;
                    filesystem::close(f);
                    continue;
                }

                char* data = clb(f, memory_globals::default_allocator());

                if (data == nullptr) {
                    log::error("resource",
                               "Could not load resouce (" "%" PRIx64 ", " "%" PRIx64 ").",
                               type,
                               name);
                }

                loaded_data[i] = data;

                filesystem::close(f);
            }
        }

        void add_loaded(char** loaded_data,
                        const StringId64_t type,
                        const StringId64_t* names,
                        const uint32_t count) {

            const uint32_t type_idx = hash::get < uint32_t > (_globals.data->_types_map, type, 0);
            auto& data_map = _globals.data->_data_map[type_idx];

            thread::spin_lock(_globals.data->add_lock);

            for (uint32_t i = 0; i < count; ++i) {
                const StringId64_t name = names[i];

                hash::set(data_map, name, loaded_data[i]);
                inc_reference(type_idx, name);

                resource_online_clb_t online_clb = hash::get < resource_online_clb_t >
                                                   (_globals.data->_online_clb_map, type, nullptr);

                online_clb(loaded_data[i]);
            }

            thread::spin_unlock(_globals.data->add_lock);
        };


        void unload(const StringId64_t type,
                    const StringId64_t* names,
                    const uint32_t count) {
            resource_unloader_clb_t clb = hash::get < resource_unloader_clb_t >
                                          (_globals.data->_unload_clb_map, type, nullptr);

            resource_offline_clb_t ofline_clb = hash::get < resource_offline_clb_t >
                                                (_globals.data->_offline_clb_map, type, nullptr);

            const uint32_t type_idx = hash::get < uint32_t > (_globals.data->_types_map, type, 0);
            auto& data_map = _globals.data->_data_map[type_idx];

            StringId64_t name = 0;

            for (uint32_t i = 0; i < count; ++i) {
                name = names[i];
                void* data = (void*) get(type, name);
                ofline_clb(data);
            }

            for (uint32_t i = 0; i < count; ++i) {
                name = names[i];

                log::debug("resource",
                           "Unload resource " "%" PRIx64 "%" PRIx64 "",
                           type, name);

                if (!dec_reference(type_idx, name)) {
                    continue;
                }

                void* data = (void*) get(type, name);
                clb(memory_globals::default_allocator(), data);

                hash::remove(data_map, name);
            }
        }

        bool can_get(const StringId64_t type,
                     const StringId64_t* names,
                     const uint32_t count) {

            CE_ASSERT("resource", hash::has(_globals.data->_types_map, type));

            const uint32_t type_idx = hash::get < uint32_t > (_globals.data->_types_map, type, 0);
            auto& data_map = _globals.data->_data_map[type_idx];

            for (uint32_t i = 0; i < count; ++i) {
                StringId64_t name = names[i];

                if (!hash::has(data_map, name)) {
                    return false;
                }
            }

            return true;
        }

        void load_now(const StringId64_t type,
                      const StringId64_t* names,
                      const uint32_t count) {
            Array < char* > loaded_data(memory_globals::default_allocator());
            array::reserve(loaded_data, count);

            load(array::begin(loaded_data), type, names, count);
            add_loaded(array::begin(loaded_data), type, names, count);
        }

        const char* get(const StringId64_t type,
                        const StringId64_t name) {

            CE_ASSERT("resource", hash::has(_globals.data->_types_map, type));

            const uint32_t type_idx = hash::get < uint32_t > (_globals.data->_types_map, type, 0);
            auto& data_map = _globals.data->_data_map[type_idx];

            if (_globals.data->autoreload) {
                if (!can_get(type, &name, 1)) {
                    log::warning("resource", "Autoreload " "%"
                    PRIx64
                    "%"
                    PRIx64
                    "", type, name);
                    load_now(type, &name, 1);
                }
            }

            return hash::get < char* > (data_map, name, nullptr);
        }

        void register_type(const StringId64_t type,
                           const resource_loader_clb_t loader_clb,
                           const resource_unloader_clb_t unloader_clb,
                           const resource_online_clb_t online_clb,
                           const resource_offline_clb_t offline_clb) {

            CE_ASSERT("resource", loader_clb != nullptr);
            CE_ASSERT("resource", unloader_clb != nullptr);
            CE_ASSERT("resource", online_clb != nullptr);
            CE_ASSERT("resource", offline_clb != nullptr);


            const uint32_t type_idx = _globals.data->type_count++;

            hash::set(_globals.data->_types_map, type, type_idx);
            hash::set(_globals.data->_load_clb_map, type, loader_clb);
            hash::set(_globals.data->_unload_clb_map, type, unloader_clb);
            hash::set(_globals.data->_online_clb_map, type, online_clb);
            hash::set(_globals.data->_offline_clb_map, type, offline_clb);
        }

    }

    namespace resource_manager_globals {
        void init() {
            log::info("resource_manager_globals", "Init");

            char* p = _globals.buffer;
            _globals.data = new(p) ResouceManagerData(memory_globals::default_allocator());
        }

        void shutdown() {
            log::info("resource_manager_globals", "Shutdown");

            _globals.data->~ResouceManagerData();
            _globals = Globals();
        }
    }
}
