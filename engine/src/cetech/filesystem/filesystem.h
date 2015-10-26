#pragma once

#include "celib/container/container_types.h"
#include "cetech/filesystem/file.h"
#include "celib/string/stringid.inl.h"

#include <ctime>

namespace cetech {
    static StringId64_t BUILD_DIR = stringid64::from_cstring("build");

    static StringId64_t CORE_DIR = stringid64::from_cstring("core");
    static StringId64_t SRC_DIR = stringid64::from_cstring("src");

    namespace filesystem {
        void setDirMap(StringId64_t name, const char* dir);

        FSFile* open(StringId64_t root, const char* path, FSFile::OpenMode mode);
        void close(FSFile* file);

        bool exists(StringId64_t root, const char* path);
        bool is_directory(StringId64_t root, const char* path);
        bool is_file(StringId64_t root, const char* path);

        const char* root_dir(StringId64_t root);

        void create_directory(StringId64_t root, const char* path);
        void delete_directory(StringId64_t root, const char* path);

        // TODO: free files string, fce or new type with destructor?
        void list_directory(StringId64_t root, const char* path, Array < char* >& files);

        void create_file(StringId64_t root, const char* path);
        void delete_file(StringId64_t root, const char* path);

        time_t file_mtime(StringId64_t root, const char* path);
    }

    namespace filesystem_globals {
        void init();
        void shutdown();
    }

}
