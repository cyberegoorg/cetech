#include <cstdio>

#include "cetech/resource_compiler/resource_compiler.h"
#include "cetech/resource_manager/resource_manager.h"
#include "cetech/resource_compiler/private/builddb.h"

#include "celib/container/types.h"
#include "celib/container/queue.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/memory/memory.h"
#include "celib/macros.h"
#include "celib/string/stringid.inl.h"
#include "cetech/thread/thread.h"
#include "sqlite3/sqlite3.h"

#include "cetech/application/application.h"
#include "cetech/cvars/cvars.h"
#include "cetech/develop/console_server.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/memorybuffer.h"
#include "rapidjson/error/en.h"


namespace cetech {
    namespace {
        using namespace resource_compiler;

        struct CompileTask {
            resource_compiler_clb_t clb;
            StringId64_t source_fs;

            char* filename;
            StringId64_t name;
            StringId64_t type;
        };

        enum {
            TASK_POOL_SIZE = 4096,
            TASK_POOL_SIZE_MASK = TASK_POOL_SIZE - 1u
        };

        struct ResouceCompilerData {
            Hash < resource_compiler_clb_t > _compile_clb_map;
            CompileTask compile_task_pool[TASK_POOL_SIZE];
            uint32_t compile_task_pool_idx;

            explicit ResouceCompilerData(Allocator& allocator) : _compile_clb_map(allocator), compile_task_pool_idx(0) {
                static_assert( 0 == (TASK_POOL_SIZE & TASK_POOL_SIZE_MASK), "TASK_POOL_SIZE must be power of two,");

                memset(compile_task_pool, 0, sizeof(CompileTask) * TASK_POOL_SIZE);
            }
        };

        struct Globals {
            static const int MEMORY = sizeof(ResouceCompilerData);
            char buffer[MEMORY];

            ResouceCompilerData* data;

            Globals() : buffer {
                0
            }, data(0) {}
        } _globals;

        CE_INLINE CompileTask& new_compile_task() {
            ResouceCompilerData* data = _globals.data;

            return data->compile_task_pool[(data->compile_task_pool_idx++) & TASK_POOL_SIZE_MASK];
        };

        CE_INLINE void calc_hash(const char* path,
                                 StringId64_t& type,
                                 StringId64_t& name) {
            const char* t = strrchr(path, '.');
            CE_ASSERT("resource_compiler", t != nullptr);

            const uint32_t sz = t - path;
            t = t + 1;

            const uint32_t len = strlen(t);

            type = stringid64::from_cstringn(t, len);
            name = stringid64::from_cstringn(path, sz);
        }

        CE_INLINE void resource_id_to_str(char* buffer,
                                          const StringId64_t& type,
                                          const StringId64_t& name) {
            std::sprintf(buffer, "%" PRIx64 "%" PRIx64, type, name);
        }

        static void compile_task(void* data) {
            CompileTask* ct = (CompileTask*)data;

            log::info("resource_compiler",
                      "[%s] Compile => (" "%" PRIx64 ", " "%" PRIx64 ").",
                      ct->filename,
                      ct->type,
                      ct->name);

            char output_filename[512] = {0};
            resource_id_to_str(output_filename, ct->type, ct->name);

            FSFile& source_file = filesystem::open(ct->source_fs, ct->filename, FSFile::READ);
            if (!source_file.is_valid()) {
                log::error("resource_compiler", "[%s] Could not open source file.", ct->filename);
                return;
            }

            FSFile& build_file = filesystem::open(BUILD_DIR, output_filename, FSFile::WRITE);

            CompilatorAPI comp(ct->filename,
                               source_file,
                               build_file);

            ct->clb(ct->filename, comp);


            char db_path[512] = {0};
            sprintf(db_path, "%s%s", filesystem::root_dir(BUILD_DIR), "build.db");

            BuildDB bdb;
            bdb.open(db_path);
            bdb.set_file(ct->filename, filesystem::file_mtime(ct->source_fs, ct->filename));
            bdb.set_file_depend(ct->filename, ct->filename);
            bdb.close();

            filesystem::close(source_file);
            filesystem::close(build_file);

            log::info("resource_compiler", "[%s] Compiled.", ct->filename );
        }

        void save_json(const char* filename,
                       const rapidjson::Document& document) {
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter < rapidjson::StringBuffer > writer(buffer);
            document.Accept(writer);

            FSFile& debug_index_file = filesystem::open(BUILD_DIR, filename, FSFile::WRITE);
            debug_index_file.write(buffer.GetString(), buffer.GetSize());
            filesystem::close(debug_index_file);
        }

        void build_config_yaml() {
            FSFile& src_config = filesystem::open(SRC_DIR, "config.yaml", FSFile::READ);
            FSFile& out_config = filesystem::open(BUILD_DIR, "config.yaml", FSFile::WRITE);

            size_t size = src_config.size();
            char data[size];
            memset(data, 0, size);
            src_config.read(data, size);
            filesystem::close(src_config);

            out_config.write(data, size);
            filesystem::close(out_config);
        }

        static void cmd_compile_all(const rapidjson::Document& in,
                                    rapidjson::Document& out) {
            CE_UNUSED(in);
            CE_UNUSED(out);
            resource_compiler::compile_all();
        }
    }

    namespace resource_compiler {
        void register_compiler(StringId64_t type,
                               resource_compiler_clb_t clb) {
            hash::set(_globals.data->_compile_clb_map, type, clb);
        }

        void compile_all() {
            filesystem::create_directory(BUILD_DIR, 0);

            BuildDB bdb;
            char db_path[512] = {0};
            sprintf(db_path, "%s%s", filesystem::root_dir(BUILD_DIR), "build.db");
            bdb.open(db_path);
            bdb.init_db();


            rapidjson::Document debug_index;
            debug_index.SetObject();

            build_config_yaml();

            static StringId64_t in_dirs[] = {CORE_DIR, SRC_DIR};

            task_manager::TaskID top_compile_task = task_manager::add_empty_begin("compiler");

            Array < char* > dir_files[] = {
                Array < char* > (memory_globals::default_allocator()),
                Array < char* > (memory_globals::default_allocator())
            };

            for (unsigned long i = 0; i < sizeof(in_dirs) / sizeof(StringId64_t); ++i) {
                StringId64_t src_dir = in_dirs[i];

                Array < char* >& files = dir_files[i];

                filesystem::list_directory(src_dir, 0, files);

                const uint32_t files_count = array::size(files);

                char resource_id_str[64] = {0};

                char db_path[512] = {0};
                sprintf(db_path, "%s%s", filesystem::root_dir(BUILD_DIR), "build.db");
                BuildDB bdb;
                bdb.open(db_path);

                for (uint32_t i = 0; i < files_count; ++i) {
                    char* filename = files[i] + strlen(filesystem::root_dir(src_dir));         /* Base path */

                    if (!strcmp(filename, "config.json")) {
                        continue;
                    }

                    uint64_t name, type = 0;
                    calc_hash(filename, type, name);
                    resource_id_to_str(resource_id_str, type, name);

                    resource_compiler_clb_t clb = hash::get < resource_compiler_clb_t >
                                                  (_globals.data->_compile_clb_map, type, nullptr);


                    if (clb == nullptr) {
                        log::warning("resource_compiler",
                                     "Resource type " "%" PRIx64 " not register compiler.",
                                     type);
                        continue;
                    }

                    debug_index.AddMember(
                        rapidjson::Value(resource_id_str, strlen(resource_id_str), debug_index.GetAllocator()),
                        rapidjson::Value(filename, strlen(filename),
                                         debug_index.GetAllocator()), debug_index.GetAllocator()
                        );

                    resource_id_str[0] = '\0';

                    if (!bdb.need_compile(src_dir, filename)) {
                        continue;
                    }

                    CompileTask& ct = new_compile_task();
                    ct.source_fs = src_dir;
                    ct.filename = filename;
                    ct.name = name;
                    ct.type = type;
                    ct.clb = clb;

                    task_manager::TaskID tid =
                        task_manager::add_begin("compile_task",
                                                compile_task,
                                                &ct,
                                                task_manager::Priority::Normal,
                                                NULL_TASK,
                                                top_compile_task);
                    task_manager::add_end(&tid, 1);
                }
            }

            task_manager::add_end(&top_compile_task, 1);
            task_manager::wait(top_compile_task);

            save_json("debug_index.json", debug_index);

            for (unsigned long i = 0; i < sizeof(in_dirs) / sizeof(StringId64_t); ++i) {
                filesystem::free_list_directory(dir_files[i]);
            }
        }
    }

    namespace resource_compiler_globals {
        void init() {
            log::info("resource_compiler_globals", "Init");

            char* p = _globals.buffer;
            _globals.data = new(p) ResouceCompilerData(memory_globals::default_allocator());

            console_server::register_command("resource_compiler.compile_all", &cmd_compile_all);
        }

        void shutdown() {
            log::info("resource_compiler_globals", "Shutdown");

            _globals.data->~ResouceCompilerData();
            _globals = Globals();
        }
    }
}
