#pragma once

#include "celib/memory/memory_types.h"

namespace cetech {
    class FileSystem;

    namespace disk_filesystem {
        FileSystem* make(Allocator& allocator, const char* root_path);
        void destroy(Allocator& allocator, FileSystem* fs);
    }
}