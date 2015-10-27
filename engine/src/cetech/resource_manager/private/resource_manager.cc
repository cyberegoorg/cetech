#include <cstdio>

#include "cetech/resource_manager/resource_manager.h"
#include "cetech/application/application.h"

#include "celib/container/container_types.h"
#include "celib/container/queue.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/memory/memory.h"
#include "celib/string/stringid.inl.h"

#include "cetech/cvars/cvars.h"
#include "cetech/platform/dir.h"
#include "cetech/platform/thread.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/memorybuffer.h"

namespace cetech {
    namespace {
        using namespace resource_manager;

        struct ResouceManagerData {
            Hash < char* > _data_map;
            Hash < uint32_t > _data_refcount_map;

            Hash < resource_loader_clb_t > _load_clb_map;
            Hash < resource_unloader_clb_t > _unload_clb_map;
            Hash < resource_online_clb_t > _online_clb_map;
            Hash < resource_offline_clb_t > _offline_clb_map;

            Spinlock add_lock;
            bool autoreload;

            ResouceManagerData(Allocator & allocator) : _data_map(allocator),
                                                        _data_refcount_map(allocator),
                                                        _load_clb_map(allocator),
                                                        _unload_clb_map(allocator),
                                                        _online_clb_map(allocator),
                                                        _offline_clb_map(allocator),
                                                        autoreload(true) {}
        };

        struct Globals {
            static const int MEMORY = sizeof(ResouceManagerData);
            char buffer[MEMORY];

            ResouceManagerData* data;

            Globals() : data(0) {}
        } _globals;

        CE_INLINE void resource_id_to_str(char* buffer, const StringId64_t& type, const StringId64_t& name) {
            std::sprintf(buffer, "%" PRIx64 "%" PRIx64, type, name);
        }

    }

    namespace resource_manager {
        CE_INLINE void inc_reference(StringId64_t type, const StringId64_t name) {
            const uint32_t counter = hash::get < uint32_t > (_globals.data->_data_refcount_map, type ^ name, 0) + 1;

            //             log_globals::log().debug("resource_manager",
            //                                      "Inc reference for (%" PRIx64 ", %" PRIx64 ") counter == %d ",
            //                                      type,
            //                                      name,
            //                                      counter);

            hash::set(_globals.data->_data_refcount_map, type ^ name, counter);
        }

        CE_INLINE bool dec_reference(StringId64_t type, const StringId64_t name) {
            const uint32_t counter = hash::get < uint32_t > (_globals.data->_data_refcount_map, type ^ name, 1) - 1;

            //             log_globals::log().debug("resource_manager",
            //                                      "Dec reference for %" PRIx64 "%" PRIx64 "." " counter == %d ",
            //                                      type,
            //                                      name,
            //                                      counter);

            hash::set(_globals.data->_data_refcount_map, type ^ name, counter);

            return counter == 0;
        }

        void load(char** loaded_data, StringId64_t type, const StringId64_t* names,
                  const uint32_t count) {
            StringId64_t name = 0;

            resource_loader_clb_t clb = hash::get < resource_loader_clb_t >
                                        (_globals.data->_load_clb_map, type, nullptr);

            if (clb == nullptr) {
                log_globals::log().error("resource_manager",
                                         "Resource type " "%" PRIx64 " not register loader.",
                                         type);
                return;
            }

            for (uint32_t i = 0; i < count; ++i) {
                name = names[i];

                log_globals::log().debug("resource_manager",
                                         "Loading resource " "%" PRIx64 "%" PRIx64 "",
                                         type, name);

                char resource_srt[32 + 1] = {0};
                resource_id_to_str(resource_srt, type, name);

                FSFile& f = filesystem::open(BUILD_DIR, resource_srt, FSFile::READ);

                if (!f.is_valid()) {
                    log_globals::log().error("resource_manager",
                                             "Could not open resouce (" "%" PRIx64 ", " "%" PRIx64 ").",
                                             type,
                                             name);
                    loaded_data[i] = nullptr;
                    filesystem::close(f);
                    continue;
                }

                char* data = clb(f, memory_globals::default_allocator());

                if (data == nullptr) {
                    log_globals::log().error("resource_manager",
                                             "Could not load resouce (" "%" PRIx64 ", " "%" PRIx64 ").",
                                             type,
                                             name);
                }

                loaded_data[i] = data;

                filesystem::close(f);
            }
        }

        void add_loaded(char** loaded_data,
                        StringId64_t type,
                        const StringId64_t* names,
                        const uint32_t count) {

            thread::spin_lock(_globals.data->add_lock);

            for (uint32_t i = 0; i < count; ++i) {
                const StringId64_t name = names[i];

                hash::set(_globals.data->_data_map, type ^ name, loaded_data[i]);
                inc_reference(type, name);

                resource_online_clb_t online_clb = hash::get < resource_online_clb_t >
                                                   (_globals.data->_online_clb_map, type, nullptr);

                if (online_clb == nullptr) {
                    log_globals::log().error("resource_manager",
                                             "Resource type " "%" PRIx64 " not register online.",
                                             type);
                    return;
                }

                online_clb(loaded_data[i]);
            }

            thread::spin_unlock(_globals.data->add_lock);
        };


        void unload(StringId64_t type, const StringId64_t* names, const uint32_t count) {
            resource_unloader_clb_t clb = hash::get < resource_unloader_clb_t >
                                          (_globals.data->_unload_clb_map, type, nullptr);

            if (clb == nullptr) {
                log_globals::log().error("resource_manager",
                                         "Resource type " "%" PRIx64 " not register unloader.",
                                         type);
                return;
            }

            resource_offline_clb_t ofline_clb = hash::get < resource_offline_clb_t >
                                                (_globals.data->_offline_clb_map, type, nullptr);

            if (ofline_clb == nullptr) {
                log_globals::log().error("resource_manager",
                                         "Resource type " "%" PRIx64 " not register offline.",
                                         type);
                return;
            }

            StringId64_t name = 0;

            for (uint32_t i = 0; i < count; ++i) {
                name = names[i];
                void* data = (void*) get(type, name);
                ofline_clb(data);
            }

            for (uint32_t i = 0; i < count; ++i) {
                name = names[i];

                log_globals::log().debug("resource_manager",
                                         "Unload resource " "%" PRIx64 "%" PRIx64 "",
                                         type, name);

                if (!dec_reference(type, name)) {
                    continue;
                }

                void* data = (void*) get(type, name);
                clb(memory_globals::default_allocator(), data);

                hash::remove(_globals.data->_data_map, type ^ name);
            }
        }

        bool can_get(StringId64_t type, StringId64_t* names, const uint32_t count) {
            StringId64_t name = 0;
            for (uint32_t i = 0; i < count; ++i) {
                name = names[i];

                if (!hash::has(_globals.data->_data_map, type ^ name)) {
                    thread::spin_unlock(_globals.data->add_lock);
                    return false;
                }
            }

            return true;
        }

        void load_now(StringId64_t type, StringId64_t* names, const uint32_t count) {
            Array < char* > loaded_data(memory_globals::default_allocator());
            array::reserve(loaded_data, count);

            load(array::begin(loaded_data), type, names, count);
            add_loaded(array::begin(loaded_data), type, names, count);
        }

        const char* get(StringId64_t type, StringId64_t name) {
            if (_globals.data->autoreload) {
                if (!can_get(type, &name, 1)) {
                    load_now(type, &name, 1);
                }
            }

            return hash::get < char* > (_globals.data->_data_map, type ^ name, nullptr);
        }

        void register_loader(StringId64_t type, resource_loader_clb_t clb) {
            hash::set(_globals.data->_load_clb_map, type, clb);
        }

        void register_unloader(StringId64_t type, resource_unloader_clb_t clb) {
            hash::set(_globals.data->_unload_clb_map, type, clb);
        }

        void register_online(StringId64_t type, resource_online_clb_t clb) {
            hash::set(_globals.data->_online_clb_map, type, clb);
        };

        void register_offline(StringId64_t type, resource_offline_clb_t clb) {
            hash::set(_globals.data->_offline_clb_map, type, clb);
        };
    }

    namespace resource_manager_globals {
        void init() {
            char* p = _globals.buffer;
            _globals.data = new(p) ResouceManagerData(memory_globals::default_allocator());
        }

        void shutdown() {
            _globals.data->~ResouceManagerData();
            _globals = Globals();
        }
    }
}