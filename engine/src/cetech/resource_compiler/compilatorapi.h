#pragma once

#include "cetech/filesystem/filesystem.h"



namespace cetech {
    struct CompilatorAPI {
        CompilatorAPI(const char* filename,
                      FSFile & resource_file,
                      FSFile & build_file);

        ~CompilatorAPI();

        bool add_dependency(const char* path);

        size_t resource_file_size();
        bool read_resource_file(char* buffer);

        void write_to_build(const void* buffer,
                            size_t size);

        struct Implementation;
        Implementation* _impl;
    };
}
