#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <memory.h>

#include <cetech/kernel/errors.h>
#include <cetech/celib/array.inl>
#include <cetech/celib/string_stream.h>

#include <cetech/kernel/os.h>


#if defined(CETECH_LINUX)

#include <dirent.h>
#include <sys/stat.h>

#endif

#if defined(CETECH_LINUX)
#define DIR_DELIM_CH '/'
#define DIR_DELIM_STR "/"
#endif

using namespace cetech;
using namespace string_stream;

//==============================================================================
// File Interface
//==============================================================================

//! Get file modified time
//! \param path File path
//! \return Modified time
uint32_t file_mtime(const char *path) {
#if defined(CETECH_LINUX)
    struct stat st;
    stat(path, &st);
    return st.st_mtime;
#endif
}

//==============================================================================
// Path Interface
//==============================================================================

void _dir_list(const char *path,
               int recursive,
               Array<char *> &tmp_files,
               struct allocator *allocator) {
#if defined(CETECH_LINUX)
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(path))) {
        return;
    }

    if (!(entry = readdir(dir))) {
        closedir(dir);
        return;
    }

    do {
        if (recursive && (entry->d_type == 4)) {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char tmp_path[1024] = {0};
            int len = 0;

            if (path[strlen(path) - 1] != '/') {
                len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s/%s/", path,
                               entry->d_name);
            } else {
                len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s%s/", path,
                               entry->d_name);
            }

            _dir_list(tmp_path, 1, tmp_files, allocator);
        } else {
            size_t size = strlen(path) + strlen(entry->d_name) + 3;
            char *new_path =
                    CETECH_ALLOCATE(allocator, char,
                                    sizeof(char) * size);

            if (path[strlen(path) - 1] != '/') {
                snprintf(new_path, size - 1, "%s/%s", path, entry->d_name);
            } else {
                snprintf(new_path, size - 1, "%s%s", path, entry->d_name);
            }

            array::push_back(tmp_files, new_path);
        }
    } while ((entry = readdir(dir)));
    closedir(dir);
#endif
}

void dir_list(const char *path,
              int recursive,
              char ***files,
              uint32_t *count,
              struct allocator *allocator) {
    Array<char *> tmp_files(allocator);

    _dir_list(path, recursive, tmp_files, allocator);

    char **new_files = CETECH_ALLOCATE(allocator, char*,
                                       sizeof(char *) * array::size(tmp_files));

    memcpy(new_files,
           array::begin(tmp_files),
           sizeof(char *) * array::size(tmp_files));

    *files = new_files;
    *count = array::size(tmp_files);
}

void dir_list_free(char **files,
                   uint32_t count,
                   struct allocator *allocator) {
    for (int i = 0; i < count; ++i) {
        CETECH_FREE(allocator, files[i]);
    }

    CETECH_FREE(allocator, files);
}


//! Create dir
//! \param path Dir path
//! \return 1 of ok else 0
int dir_make(const char *path) {
#if defined(CETECH_LINUX)
    struct stat st;
    const int mode = 0775;

    if (stat(path, &st) != 0) {
        if (mkdir(path, mode) != 0 && errno != EEXIST) {
            return 0;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return 0;
    }

    return 1;
#endif
}

//! Create dir path
//! \param path Path
//! \return 1 of ok else 0
int dir_make_path(const char *path) {
#if defined(CETECH_LINUX)
    char *pp;
    char *sp;
    int status = 1;
    char *copypath = strdup(path);

    pp = copypath;
    while (status == 1 && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            *sp = '\0';
            status = dir_make(copypath);
            *sp = '/';
        }

        pp = sp + 1;
    }

    if (status == 1) {
        status = dir_make(path);
    }

    free(copypath);
    return status;
#endif
}


//! Get filename from path
//! \param path Path
//! \return Filename
const char *path_filename(const char *path) {
    char *ch = strrchr(path, DIR_DELIM_CH);
    return ch != NULL ? ch + 1 : path;
}

//! Get file basename (filename without extension)
//! \param path Path
//! \param out Out basename
//! \param size
void path_basename(const char *path,
                   char *out,
                   size_t size) {
    const char *filename = path_filename(path);
    const char *ch = strrchr(filename, '.');

    if (ch == NULL) {
        memcpy(out, filename, strlen(filename));
        return;
    }

    const size_t basename_len = (ch - filename) / sizeof(char);
    memcpy(out, filename, basename_len);
}

void path_dir(char *out,
              size_t size,
              const char *path) {
#if defined(CETECH_LINUX)
    char *ch = strrchr(path, DIR_DELIM_CH);

    if (ch != NULL) {
        size_t len = ch - path;
        memcpy(out, path, len);
        out[len] = '\0';
    }
#endif
}

//! Get file extension
//! \param path Path
//! \return file extension
const char *path_extension(const char *path) {
    const char *filename = path_filename(path);
    const char *ch = strrchr(filename, '.');

    if (ch == NULL) {
        return NULL;
    }

    return ch + 1;
}

char *path_join(struct allocator *allocator,
                uint32_t count,
                ...) {

    Buffer buffer(allocator);

    va_list arguments;
    va_start (arguments, count);

    buffer << va_arg (arguments, const char*);

    for (int i = 1; i < count; ++i) {
        buffer << DIR_DELIM_STR;
        buffer << va_arg (arguments, const char*);
    }

    va_end (arguments);

    c_str(buffer);
    char *data = buffer._data;
    buffer._data = NULL;

    return data;
}
