#include "cetech/resource_compiler/compilatorapi.h"
#include "cetech/resource_compiler/private/builddb.h"

#include "celib/log/log.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/memorybuffer.h"
#include "rapidjson/error/en.h"

namespace cetech {
    struct CompilatorAPI::Implementation {
        BuildDB bdb;
        FSFile& resource_file;
        FSFile& build_file;
        const char* filename;

        Implementation(const char* filename, FSFile & resource_file,
                       FSFile & build_file) : resource_file(resource_file), build_file(build_file), filename(filename) {
            char db_path[512] = {0};
            sprintf(db_path, "%s%s", filesystem::root_dir(BUILD_DIR), "build.db");
            bdb.open(db_path);

        }

        bool add_dependency(const char* path) {
            bdb.set_file(path, filesystem::file_mtime(SRC_DIR, path));
            bdb.set_file_depend(filename, path);
            return true;
        }

        size_t resource_file_size() {
            return resource_file.size();
        }

        bool read_resource_file(char* buffer) {
            size_t sz_in = resource_file.size();
            resource_file.read(buffer, sz_in);
            return true;
        }

        void write_to_build(const void* buffer,
                            size_t size) {
            build_file.write(buffer, size);
        }

    };

    CompilatorAPI::CompilatorAPI(const char* filename,
                                 FSFile& resource_file,
                                 FSFile& build_file) {
        _impl = new CompilatorAPI::Implementation(filename,
                                                  resource_file,
                                                  build_file);
    }

    CompilatorAPI::~CompilatorAPI() {
        delete _impl;
    }

    bool CompilatorAPI::add_dependency(const char* path) {
        return _impl->add_dependency(path);
    }

    bool CompilatorAPI::read_resource_file(char* buffer) {
        return _impl->read_resource_file(buffer);
    }

    size_t CompilatorAPI::resource_file_size() {
        return _impl->resource_file_size();
    }

    void CompilatorAPI::write_to_build(const void* buffer,
                                       size_t size) {
        return _impl->write_to_build(buffer, size);
    }
}
