#include "celib/platforms/file/file.h"

#include <time.h>
#include <sys/stat.h>

namespace cetech {

    namespace file {
        time_t mtime(const char* path) {
            struct stat st;
            stat(path, &st);
            return st.st_mtime;
        }
    }
}