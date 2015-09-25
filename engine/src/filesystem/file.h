#pragma once

#include <cstring>

namespace cetech {
    class File {
        public:
            enum OpenMode {
                READ = 1,
                WRITE = 2
            };

            virtual ~File() {};

            virtual bool is_valid() = 0;

            virtual void seek(size_t position) = 0;
            virtual void seek_to_end() = 0;
            virtual void skip(size_t bytes) = 0;

            virtual void read(void* buffer, size_t size) = 0;
            virtual void write(const void* buffer, size_t size) = 0;

            virtual void flush() = 0;

            virtual bool end_of_file() = 0;

            virtual size_t size() = 0;
            virtual size_t position() = 0;
    };
}