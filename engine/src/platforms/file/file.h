#pragma once

#include "common/platform/defines.h"

#include "common/container/container_types.h"
#include "common/math/math_types.h"

#include "platforms/file/file_types.h"

namespace cetech {
    namespace file {
        enum SeekWhence {
            SW_SEEK_SET = 1,
            SW_SEEK_CUR,
            SW_SEEK_END
        };

        File from_file(const char* path, const char* mode);

        bool is_null(const File& f);

        int close(const File& f);
        size_t read(const File& f, void* ptr, size_t size, size_t maxnum);
        size_t write(const File& f, const void* ptr, size_t size, size_t num);

        int64_t seek(const File& f, int64_t offset, SeekWhence whence);
        int64_t tell(const File& f);

        size_t size(const File& f);
    }
}