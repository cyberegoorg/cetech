#ifndef CELIB_PATH_H
#define CELIB_PATH_H

#include <stdio.h>

#include "../memory/memory.h"
#include "../string/string.h"
#include "../types.h"

#if defined(CELIB_LINUX)
#define DIR_DELIM_CH '/'
#define DIR_DELIM_STR "/"
#endif

//! Get filename from path
//! \param path Path
//! \return Filename
static const char *celib_path_filename(const char *path) {
    char *ch = strrchr(path, DIR_DELIM_CH);
    return ch != NULL ? ch + 1 : path;
}

//! Get file basename (filename without extension)
//! \param path Path
//! \param out Out basename
//! \param size
static void celib_path_basename(const char *path,
                                char *out,
                                size_t size) {
    const char *filename = celib_path_filename(path);
    const char *ch = strrchr(filename, '.');

    if (ch == NULL) {
        memory_copy(out, filename, str_lenght(filename));
        return;
    }

    const size_t basename_len = (ch - filename) / sizeof(char);
    memory_copy(out, filename, basename_len);
}

static void celib_path_dir(char *out,
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
static const char *celib_path_extension(const char *path) {
    const char *filename = celib_path_filename(path);
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
static i64 celib_path_join(char *result,
                           u64 maxlen,
                           const char *base_path,
                           const char *path) {
    return snprintf(result, maxlen, "%s" DIR_DELIM_STR "%s", base_path, path);
}


#endif //CELIB_PATH_H
