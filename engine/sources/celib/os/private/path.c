#include <memory.h>

#include <celib/memory/memory.h>
#include <celib/string/string.h>
#include <stdio.h>
#include <celib/containers/array.h>

const char *os_path_filename(const char *path) {
    char *ch = strrchr(path, '/');
    return ch != NULL ? ch + 1 : path;
}

void os_path_basename(const char *path, char *out, size_t size) {
    const char *filename = os_path_filename(path);
    const char *ch = strrchr(filename, '.');

    if (ch == NULL) {
        memory_copy(out, filename, str_lenght(filename));
        return;
    }

    const size_t basename_len = (ch - filename) / sizeof(char);
    memory_copy(out, filename, basename_len);
}

const char *os_path_extension(const char *path) {
    const char *filename = os_path_filename(path);
    const char *ch = strrchr(filename, '.');

    if (ch == NULL) {
        return NULL;
    }

    return ch + 1;
}

#define DIR_DELIM "/"

i64 os_path_join(char *result, u64 maxlen, const char *base_path, const char *path) {
    return snprintf(result, maxlen, "%s" DIR_DELIM "%s", base_path, path);
}
void os_path_dir(char *out,
                 size_t size,
                 const char *path) {
    char *ch = strrchr(path, '/');

    if (ch != NULL) {
        size_t len = ch - path;
        memory_copy(out, path, len);
        out[len] = '\0';
    }
}