#pragma once

#include "common/container/container_types.h"
#include "filesystem/file.h"

namespace cetech {
    class FileSystem {
        public:
            virtual ~FileSystem() {};

            virtual FSFile* open(const char* path, FSFile::OpenMode mode) = 0;
            virtual void close(FSFile* file) = 0;

            virtual bool exists(const char* path) = 0;
            virtual bool is_directory(const char* path) = 0;
            virtual bool is_file(const char* path) = 0;

            virtual void create_directory(const char* path) = 0;
            virtual void delete_directory(const char* path) = 0;

            // TODO: free files string, fce or new type with destructor?
            virtual void list_directory(const char* path, Array < char* >& files) = 0;

            virtual void create_file(const char* path) = 0;
            virtual void delete_file(const char* path) = 0;
    };
}