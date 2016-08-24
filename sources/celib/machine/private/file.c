#include <time.h>
#include <sys/stat.h>

time_t file_mtime(const char* path) {
    struct stat st;
    stat(path, &st);
    return st.st_mtime;
}