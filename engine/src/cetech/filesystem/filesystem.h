#pragma once

#include "celib/container_types.h"
#include "cetech/filesystem/file.h"
#include "celib/string/stringid.inl.h"

#include <ctime>

namespace cetech {
    static StringId64_t BUILD_DIR = stringid64::from_cstring("build");
    static StringId64_t CORE_DIR = stringid64::from_cstring("core");
    static StringId64_t SRC_DIR = stringid64::from_cstring("src");

    namespace filesystem {
        void map_root_dir(StringId64_t name,
                          const char* dir);

        FSFile& open(StringId64_t root,
                     const char* path,
                     FSFile::OpenMode mode);

        void close(FSFile& file);

        const char* root_dir(const StringId64_t root);

        void create_directory(const StringId64_t root,
                              const char* path);

        void delete_directory(const StringId64_t root,
                              const char* path);

        // TODO: free files string, fce or new type with destructor?
        void list_directory(const StringId64_t root,
                            const char* path,
                            Array < char* >& files);

        void free_list_directory(Array < char* >& files);

        void create_file(const StringId64_t root,
                         const char* path);

        time_t file_mtime(const StringId64_t root,
                          const char* path);
    }

    namespace filesystem_globals {
        void init();
        void shutdown();
    }

}
