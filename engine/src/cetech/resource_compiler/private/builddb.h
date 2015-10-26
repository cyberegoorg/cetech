#pragma once

#include <ctime>
#include "celib/string/stringid_types.h"

struct sqlite3;

namespace cetech {
    class FileSystem;

    struct BuildDB {
        BuildDB();
        ~BuildDB();

        bool init_db();
        void open(const char* db_path);
        void close();

        void set_file(const char* filename, time_t mtime);
        void set_file_depend(const char* filename, const char* depend_on);

        bool need_compile(StringId64_t root, const char* filename);

        sqlite3* _db;
    };
}
