#include <stdio.h>
#include <memory.h>
#include <cetech/memory/allocator.h>
#include <cetech/string/string.h>


#include "cetech/types.h"

#if defined(CELIB_LINUX)
#define DIR_DELIM_CH '/'
#define DIR_DELIM_STR "/"
#endif

//! Get filename from path
//! \param path Path
//! \return Filename
const char *cel_path_filename(const char *path) {
    char *ch = strrchr(path, DIR_DELIM_CH);
    return ch != NULL ? ch + 1 : path;
}

//! Get file basename (filename without extension)
//! \param path Path
//! \param out Out basename
//! \param size
void cel_path_basename(const char *path,
                       char *out,
                       size_t size) {
    const char *filename = cel_path_filename(path);
    const char *ch = strrchr(filename, '.');

    if (ch == NULL) {
        memory_copy(out, filename, cel_strlen(filename));
        return;
    }

    const size_t basename_len = (ch - filename) / sizeof(char);
    memory_copy(out, filename, basename_len);
}

void cel_path_dir(char *out,
                  size_t size,
                  const char *path) {
#if defined(CELIB_LINUX)
    char *ch = strrchr(path, DIR_DELIM_CH);

    if (ch != NULL) {
        size_t len = ch - path;
        memory_copy(out, path, len);
        out[len] = '\0';
    }
#endif
}

//! Get file extension
//! \param path Path
//! \return file extension
const char *cel_path_extension(const char *path) {
    const char *filename = cel_path_filename(path);
    const char *ch = strrchr(filename, '.');

    if (ch == NULL) {
        return NULL;
    }

    return ch + 1;
}

//! Join path
//! \param result Output path
//! \param maxlen Result len
//! \param base_path Base path
//! \param path Path
//! \return Result path len
int64_t cel_path_join(char *result,
                  uint64_t maxlen,
                  const char *base_path,
                  const char *path) {
    return snprintf(result, maxlen, "%s" DIR_DELIM_STR "%s", base_path, path);
}
