#include <cstdio>

#include "resource_compiler.h"

#include "celib/container/container_types.h"
#include "celib/container/queue.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/memory/memory.h"
#include "celib/string/stringid.inl.h"
#include "celib/platforms/dir/dir.h"
#include "celib/platforms/thread/thread.h"

#include "cetech/application/application.h"
#include "cetech/cvars/cvars.h"
#include "cetech/filesystem/disk_filesystem.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/memorybuffer.h"

namespace cetech {
    class ResourceCompilerImplementation final : public ResourceCompiler {
        public:
            friend class ResourceCompiler;

            struct CompileTask {
                FileSystem* source_fs;
                FileSystem* out_fs;

                char* filename;
                uint64_t name, type;

                resource_compiler_clb_t clb;
            };

            FileSystem* _build_fs;
            Hash < resource_compiler_clb_t > _compile_clb_map;

            ResourceCompilerImplementation(FileSystem * build_fs, Allocator & allocator) : _build_fs(build_fs),
                                                                                           _compile_clb_map(allocator) {}


            static void compile_task(void* data) {
                CompileTask* ct = (CompileTask*)data;

                log::info("resource_manager",
                          "Compile \"%s\" => (" "%" PRIx64 ", " "%" PRIx64 ").",
                          ct->filename,
                          ct->type,
                          ct->name);

                char output_filename[512] = {0};
                resource_id_to_str(output_filename, ct->type, ct->name);

                FSFile* f_in;
                f_in = ct->source_fs->open(ct->filename, FSFile::READ);
                if (!f_in->is_valid()) {
                    log::error("resource_manager", "Could not open source file \"%s\"", ct->filename);
                    return;
                }

                FSFile* f_out = ct->out_fs->open(output_filename, FSFile::WRITE);

                ct->clb(f_in, f_out);

                ct->source_fs->close(f_in);
                ct->out_fs->close(f_out);

                //MAKE_DELETE(memory_globals::default_allocator(), CompileTask, data);
            }

            TaskManager::TaskID compile(FileSystem* source_fs, rapidjson::Document& debug_index) {
                Array < char* > files(memory_globals::default_allocator());
                source_fs->list_directory(source_fs->root_dir(), files);

                TaskManager& tm = application_globals::app().task_manager();
                TaskManager::TaskID top_compile_task = tm.add_empty_begin(0);

                const uint32_t files_count = array::size(files);

                char resource_id_str[64] = {0};
                for (uint32_t i = 0; i < files_count; ++i) {
                    const char* filename = files[i] + strlen(source_fs->root_dir()); /* Base path */

                    if (!strcmp(filename, "config.json")) {
                        continue;
                    }

                    uint64_t name, type = 0;
                    calc_hash(filename, type, name);

                    resource_compiler_clb_t clb = hash::get < resource_compiler_clb_t >
                                                  (this->_compile_clb_map, type, nullptr);

                    if (clb == nullptr) {
                        log::error("resource_manager", "Resource type " "%" PRIx64 " not register compiler.", type);
                        continue;
                    }

                    resource_id_to_str(resource_id_str, type, name);
                    debug_index.AddMember(rapidjson::Value(resource_id_str, strlen(resource_id_str),
                                                           debug_index.GetAllocator()),
                                          rapidjson::Value(filename, strlen(filename),
                                                           debug_index.
                                                           GetAllocator()), debug_index.GetAllocator());
                    resource_id_str[0] = '\0';

                    // TODO: Compile Task Pool, reduce alloc free, ringbuffer?
                    CompileTask* ct = MAKE_NEW(memory_globals::default_allocator(), CompileTask);
                    ct->source_fs = source_fs;
                    ct->out_fs = _build_fs;
                    ct->filename = strdup(filename);
                    ct->name = name;
                    ct->type = type;
                    ct->clb = clb;

                    TaskManager::TaskID tid = tm.add_begin(compile_task, ct, 0, NULL_TASK, top_compile_task);
                    tm.add_end(&tid, 1);
                }

                dir::listdir_free(files);

                tm.add_end(&top_compile_task, 1);
                return top_compile_task;
            }

            virtual void register_compiler(StringId64_t type, resource_compiler_clb_t clb) final {
                hash::set(this->_compile_clb_map, type, clb);
            }

            virtual void compile_all_resource() final {
                TaskManager& tm = application_globals::app().task_manager();

                rapidjson::Document debug_index;
                debug_index.SetObject();

                FileSystem* source_fs = disk_filesystem::make(
                    memory_globals::default_allocator(), cvars::rm_source_dir.value_str);

                FileSystem* core_fs = disk_filesystem::make(
                    memory_globals::default_allocator(), cvars::compiler_core_path.value_str);

                dir::mkpath(_build_fs->root_dir());

                FSFile* src_config = source_fs->open("config.json", FSFile::READ);
                FSFile* out_config = _build_fs->open("config.json", FSFile::WRITE);

                size_t size = src_config->size();
                char data[size + 1] = {0};
                src_config->read(data, size);
                source_fs->close(src_config);

                out_config->write(data, size + 1);
                _build_fs->close(out_config);

                TaskManager::TaskID compile_tid = compile(source_fs, debug_index);
                tm.wait(compile_tid);

                compile_tid = compile(core_fs, debug_index);
                tm.wait(compile_tid);

                rapidjson::StringBuffer buffer;
                rapidjson::PrettyWriter < rapidjson::StringBuffer > writer(buffer);
                debug_index.Accept(writer);

                FSFile* debug_index_file = _build_fs->open("debug_index.json", FSFile::WRITE);
                debug_index_file->write(buffer.GetString(), buffer.GetSize());
                _build_fs->close(debug_index_file);

                disk_filesystem::destroy(memory_globals::default_allocator(), source_fs);
                disk_filesystem::destroy(memory_globals::default_allocator(), core_fs);
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

    ResourceCompiler* ResourceCompiler::make(Allocator& allocator, FileSystem* fs) {
        return MAKE_NEW(allocator, ResourceCompilerImplementation, fs, allocator);
    }

    void ResourceCompiler::destroy(Allocator& allocator, ResourceCompiler* rm) {
        MAKE_DELETE(memory_globals::default_allocator(), ResourceCompiler, rm);
    }
}