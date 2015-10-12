#pragma once

#include "celib/defines.h"
#include "celib/container/container_types.h"

namespace cetech {
    namespace dir {
        bool mkdir(const char* path);
        bool mkpath(const char* path);

        void listdir(const char* name, Array < char* >& files);
        void listdir_free(Array < char* >& files);
    }
}