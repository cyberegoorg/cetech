#pragma once

#include <ctime>

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

        bool need_compile(const char* filename, FileSystem* source_fs);

        sqlite3* _db;
    };
}
