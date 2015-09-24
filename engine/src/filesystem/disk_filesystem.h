#pragma once

#include "common/memory/memory_types.h"

namespace cetech {
    class FileSystem;

    namespace disk_filesystem {
        FileSystem* make(Allocator& alocator, const char* root_path);
        void destroy(Allocator& alocator, FileSystem* fs);
    }
}