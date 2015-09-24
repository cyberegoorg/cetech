#pragma once

#include "filesystem/file.h"

namespace cetech {
    class FileSystem {
        public:
            virtual ~FileSystem() {};

            virtual File* open(const char* path, File::OpenType mode) = 0;
            virtual void close(File* file) = 0;

            virtual bool exists(const char* path) = 0;
            virtual bool is_directory(const char* path) = 0;
            virtual bool is_file(const char* path) = 0;

            virtual void create_directory(const char* path) = 0;
            virtual void delete_directory(const char* path) = 0;

            virtual void create_file(const char* path) = 0;
            virtual void delete_file(const char* path) = 0;
    };
}