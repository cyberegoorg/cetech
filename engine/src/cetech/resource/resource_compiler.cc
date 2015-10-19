#include <cstdio>

#include "resource_compiler.h"

#include "celib/container/container_types.h"
#include "celib/container/queue.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/memory/memory.h"
#include "celib/string/stringid.inl.h"
#include "celib/platform/dir.h"
#include "celib/platform/thread.h"
#include "celib/sqlite/sqlite3.h"

#include "cetech/application/application.h"
#include "cetech/cvars/cvars.h"
#include "cetech/filesystem/disk_filesystem.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/memorybuffer.h"
#include "rapidjson/error/en.h"

namespace cetech {
    struct CompileTask {
        ResourceCompiler::resource_compiler_clb_t clb;
        FileSystem* source_fs;
        FileSystem* out_fs;

        rapidjson::Document* dependency_index;

        char* filename;
        StringId64_t name;
        StringId64_t type;
    };

    enum {
        TASK_POOL_SIZE = 4096
    };

    static CompileTask compile_task_pool[TASK_POOL_SIZE];
    static uint32_t compile_task_pool_idx;

    CE_INLINE CompileTask& new_compile_task() {
        return compile_task_pool[(compile_task_pool_idx++) % TASK_POOL_SIZE];
    };

    bool Compilator::resource_to_json(rapidjson::Document& document) {
        /* parse resouce json */
        size_t sz_in = resource_file->size();
        char tmp[4096] = {0};
        resource_file->read(tmp, sz_in);

        document.Parse(tmp);
        if (document.HasParseError()) {
            log::error("resource_package.compiler", "Parse error: %s", GetParseError_En(
                           document.GetParseError()), document.GetErrorOffset());
            return false;
        }

        return true;
    }

    CE_INLINE void calc_hash(const char* path, StringId64_t& type, StringId64_t& name) {
        const char* t = strrchr(path, '.');
        CE_CHECK_PTR(t);

        const uint32_t sz = t - path;
        t = t + 1;

        const uint32_t len = strlen(t);

        type = stringid64::from_cstringn(t, len);
        name = stringid64::from_cstringn(path, sz);
    }

    CE_INLINE void resource_id_to_str(char* buffer, const StringId64_t& type, const StringId64_t& name) {
        std::sprintf(buffer, "%" PRIx64 "%" PRIx64, type, name);
    }

    class ResourceCompilerImplementation final : public ResourceCompiler {
        public:
            friend class ResourceCompiler;

            FileSystem* _build_fs;
            Hash < resource_compiler_clb_t > _compile_clb_map;

            ResourceCompilerImplementation(FileSystem * build_fs, Allocator & allocator) : _build_fs(build_fs),
                                                                                           _compile_clb_map(allocator) {}


            static void compile_task(void* data) {
                CompileTask* ct = (CompileTask*)data;

                log::info("resource_compiler",
                          "Compile \"%s\" => (" "%" PRIx64 ", " "%" PRIx64 ").",
                          ct->filename,
                          ct->type,
                          ct->name);

                char output_filename[512] = {0};
                resource_id_to_str(output_filename, ct->type, ct->name);

                FSFile* f_in;
                f_in = ct->source_fs->open(ct->filename, FSFile::READ);
                if (!f_in->is_valid()) {
                    log::error("resource_compiler", "Could not open source file \"%s\"", ct->filename);
                    return;
                }

                FSFile* f_out = ct->out_fs->open(output_filename, FSFile::WRITE);

                sqlite3 *db;
                char *zErrMsg = 0;
                int  rc;
                char db_path[512] = {0};
                sprintf(db_path, "%s%s", ct->out_fs->root_dir(), "build.db");
                sqlite3_open(db_path, &db);

                /* Create SQL statement */
                const char* sql = "CREATE TABLE COMPANY("  \
                        "ID INT PRIMARY KEY     NOT NULL," \
                        "NAME           TEXT    NOT NULL," \
                        "AGE            INT     NOT NULL," \
                        "ADDRESS        CHAR(50)," \
                        "SALARY         REAL );";

                /* Execute SQL statement */
                rc = sqlite3_exec(db, sql, NULL , 0, &zErrMsg);
                if( rc != SQLITE_OK ){
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                }else{
                    fprintf(stdout, "Table created successfully\n");
                }
                sqlite3_close(db);

                Compilator comp(ct->source_fs, ct->out_fs, f_in);

                ct->clb(ct->filename, f_in, f_out, comp);

                ct->source_fs->close(f_in);
                ct->out_fs->close(f_out);

                log::info("resource_compiler",
                          "Compiled \"%s\" => (" "%" PRIx64 ", " "%" PRIx64 ").",
                          ct->filename,
                          ct->type,
                          ct->name);

                //MAKE_DELETE(memory_globals::default_allocator(), CompileTask, data);
                
                sqlite3_close(db);
            }

            TaskManager::TaskID compile(FileSystem* source_fs,
                                        rapidjson::Document& debug_index,
                                        rapidjson::Document& build_index,
                                        rapidjson::Document& dependency_index) {
                Array < char* > files(memory_globals::default_allocator());
                source_fs->list_directory(source_fs->root_dir(), files);

                TaskManager& tm = application_globals::app().task_manager();
                TaskManager::TaskID top_compile_task = tm.add_empty_begin(0);

                const uint32_t files_count = array::size(files);

                sqlite3 *db;
                char db_path[512] = {0};
                sprintf(db_path, "%s%s", _build_fs->root_dir(), "build.db");

                sqlite3_open(db_path, &db);
                sqlite3_close(db);

                char resource_id_str[64] = {0};
                for (uint32_t i = 0; i < files_count; ++i) {
                    const char* filename = files[i] + strlen(source_fs->root_dir()); /* Base path */

                    if (!strcmp(filename, "config.json")) {
                        continue;
                    }

                    uint64_t name, type = 0;
                    calc_hash(filename, type, name);
                    resource_id_to_str(resource_id_str, type, name);

                    time_t source_mt = source_fs->file_mtime(filename);
                    bool need_compile = true;
                    if (build_index.HasMember(resource_id_str)) {
                        time_t build_mt = build_index[resource_id_str].GetInt();
                        if (source_mt == build_mt) {
                            need_compile = false;
                        } else {
                            build_index[resource_id_str].SetInt(source_mt);
                        }
                    } else {
                        build_index.AddMember(
                            rapidjson::Value(resource_id_str, strlen(resource_id_str), build_index.GetAllocator()),
                            rapidjson::Value(source_mt), build_index.GetAllocator()
                            );
                    }

                    resource_compiler_clb_t clb = hash::get < resource_compiler_clb_t >
                                                  (this->_compile_clb_map, type, nullptr);


                    if (clb == nullptr) {
                        log::warning("resource_compiler", "Resource type " "%" PRIx64 " not register compiler.", type);
                        continue;
                    }

                    debug_index.AddMember(
                        rapidjson::Value(resource_id_str, strlen(resource_id_str), debug_index.GetAllocator()),
                        rapidjson::Value(filename, strlen(filename),
                                         debug_index.GetAllocator()), debug_index.GetAllocator()
                        );

                    resource_id_str[0] = '\0';

                    if (!need_compile) {
                        continue;
                    }

                    CompileTask& ct = new_compile_task();
                    ct.source_fs = source_fs;
                    ct.out_fs = _build_fs;
                    ct.filename = strdup(filename);
                    ct.name = name;
                    ct.type = type;
                    ct.clb = clb;
                    ct.dependency_index = &dependency_index;

                    TaskManager::TaskID tid = tm.add_begin(compile_task, &ct, 0, NULL_TASK, top_compile_task);
                    tm.add_end(&tid, 1);
                }

                dir::listdir_free(files);

                tm.add_end(&top_compile_task, 1);
                return top_compile_task;
            }

            virtual void register_compiler(StringId64_t type, resource_compiler_clb_t clb) final {
                hash::set(this->_compile_clb_map, type, clb);
            }

            void save_json(const char* filename, const rapidjson::Document& document) {
                rapidjson::StringBuffer buffer;
                rapidjson::PrettyWriter < rapidjson::StringBuffer > writer(buffer);
                document.Accept(writer);

                FSFile* debug_index_file = _build_fs->open(filename, FSFile::WRITE);
                debug_index_file->write(buffer.GetString(), buffer.GetSize());
                _build_fs->close(debug_index_file);
            }

            void build_config_json(FileSystem* source_fs, FileSystem* build_fs) {
                FSFile* src_config = source_fs->open("config.json", FSFile::READ);
                FSFile* out_config = build_fs->open("config.json", FSFile::WRITE);

                size_t size = src_config->size();
                char data[size];
                memset(data, 0, size);
                src_config->read(data, size);
                source_fs->close(src_config);

                out_config->write(data, size);
                build_fs->close(out_config);
            }

            void load_debug_index(rapidjson::Document& build_index) {
                FSFile* build_index_file = _build_fs->open("build_index.json", FSFile::READ);

                if (!build_index_file->is_valid()) {
                    _build_fs->close(build_index_file);
                    build_index.SetObject();
                    return;
                }

                size_t size = build_index_file->size();
                char data[size + 1];
                memset(data, 0, size + 1);
                build_index_file->read(data, size);

                build_index.Parse(data);
                if (build_index.HasParseError()) {
                    log::error("resouce_compiler", "debug_index.json parse error: %s", GetParseError_En(
                                   build_index.GetParseError()), build_index.GetErrorOffset());
                }

                _build_fs->close(build_index_file);
            }

            virtual void compile_all_resource() final {
                TaskManager& tm = application_globals::app().task_manager();

                FileSystem* source_fs = disk_filesystem::make(
                    memory_globals::default_allocator(), cvars::rm_source_dir.value_str);

                FileSystem* core_fs = disk_filesystem::make(
                    memory_globals::default_allocator(), cvars::compiler_core_path.value_str);

                rapidjson::Document debug_index;
                debug_index.SetObject();

                rapidjson::Document dependency_index;
                dependency_index.SetObject();

                rapidjson::Document build_index;
                load_debug_index(build_index);

                dir::mkpath(_build_fs->root_dir());

                build_config_json(source_fs, _build_fs);

                TaskManager::TaskID compile_tid = compile(source_fs, debug_index, build_index, dependency_index);
                tm.wait(compile_tid);

                compile_tid = compile(core_fs, debug_index, build_index, dependency_index);
                tm.wait(compile_tid);

                save_json("debug_index.json", debug_index);
                save_json("build_index.json", build_index);
                save_json("dependency_index.json", dependency_index);

                disk_filesystem::destroy(memory_globals::default_allocator(), source_fs);
                disk_filesystem::destroy(memory_globals::default_allocator(), core_fs);
            }
    };

    ResourceCompiler* ResourceCompiler::make(Allocator& allocator, FileSystem* fs) {
        return MAKE_NEW(allocator, ResourceCompilerImplementation, fs, allocator);
    }

    void ResourceCompiler::destroy(Allocator& allocator, ResourceCompiler* rm) {
        MAKE_DELETE(allocator, ResourceCompiler, rm);
    }
}
