
#include <glob.h>
#include <fnmatch.h>
#include <stdlib.h>

#include <celib/platform.h>

#include <errno.h>

#if CE_PLATFORM_LINUX
#include <dirent.h>
#include <sys/stat.h>
#include <malloc.h>
#endif

#if CE_PLATFORM_OSX

#include <dirent.h>
#include <sys/stat.h>

#endif

#include <celib/memory/allocator.h>

#include <celib/os/path.h>
#include <celib/os/vio.h>
#include "celib/containers/buffer.h"


#if CE_PLATFORM_LINUX || CE_PLATFORM_OSX
#define DIR_DELIM_CH '/'
#define DIR_DELIM_STR "/"
#endif

//==============================================================================
// File Interface
//==============================================================================

//! Get file modified time
//! \param path File path
//! \return Modified time
uint32_t file_mtime(const char *path) {
    struct stat st;
    stat(path, &st);
    return st.st_mtime;
}

//==============================================================================
// Path Interface
//==============================================================================

void _dir_list(const char *path,
               const char **patern,
               uint32_t patern_n,
               int recursive,
               int only_dir,
               char ***tmp_files,
               struct ce_alloc_t0 *allocator) {
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
        if (entry->d_type == 4) {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char tmp_path[1024] = {};
            int len = 0;

            if (path[strlen(path) - 1] != '/') {
                len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s/%s/", path,
                               entry->d_name);
            } else {
                len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s%s/", path,
                               entry->d_name);
            }

            tmp_path[len] = '\0';

            if (only_dir) {
                char *new_path = CE_ALLOC(allocator, char,
                                          sizeof(char) * (len + 1));
                memcpy(new_path, tmp_path, len + 1);
                ce_array_push(*tmp_files, new_path, allocator);
            }

            if (recursive) {
                _dir_list(tmp_path, patern, patern_n, recursive,
                          only_dir, tmp_files, allocator);
            }

        } else if (!only_dir) {
            size_t size = strlen(path) + strlen(entry->d_name) + 3;
            char *new_path =
                    CE_ALLOC(allocator, char, sizeof(char) * size);

            if (path[strlen(path) - 1] != '/') {
                snprintf(new_path, size - 1, "%s/%s", path, entry->d_name);
            } else {
                snprintf(new_path, size - 1, "%s%s", path, entry->d_name);
            }

            for (int i = 0; i < patern_n; ++i) {
                if (0 == fnmatch(patern[i], new_path, 0)) {
                    ce_array_push(*tmp_files, new_path, allocator);
                    break;
                }
            }
        }

    } while ((entry = readdir(dir)));
    closedir(dir);
}

void _dir_list2(const char *path,
                const char *patern,
                int recursive,
                int only_dir,
                void(*on_item)(const char *filename)) {
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
        if (entry->d_type == 4) {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char tmp_path[1024] = {};
            int len = 0;

            if (path[strlen(path) - 1] != '/') {
                len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s/%s/", path,
                               entry->d_name);
            } else {
                len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s%s/", path,
                               entry->d_name);
            }

            if (only_dir) {
                tmp_path[len] = '\0';
                on_item(tmp_path);
            }

            if (recursive) {
                _dir_list2(tmp_path, patern, recursive, only_dir, on_item);
            }

        } else if (!only_dir) {
            size_t size = strlen(path) + strlen(entry->d_name) + 3;
            char new_path[size];

            if (path[strlen(path) - 1] != '/') {
                snprintf(new_path, size - 1, "%s/%s", path, entry->d_name);
            } else {
                snprintf(new_path, size - 1, "%s%s", path, entry->d_name);
            }

            if (0 != fnmatch(patern, new_path, 0)) {
                continue;
            }

            on_item(new_path);
        }

    } while ((entry = readdir(dir)));
    closedir(dir);
}

void dir_list(const char *path,
              const char **patern,
              uint32_t patern_n,
              int recursive,
              int only_dir,
              char ***files,
              uint32_t *count,
              struct ce_alloc_t0 *allocator) {

    char **tmp_files = NULL;

    _dir_list(path, patern, patern_n, recursive, only_dir, &tmp_files, allocator);

    char **new_files = CE_ALLOC(allocator, char*,
                                sizeof(char *) * ce_buffer_size(tmp_files));

    memcpy(new_files, tmp_files,
           sizeof(char *) * ce_buffer_size(tmp_files));

    *files = new_files;
    *count = ce_buffer_size(tmp_files);
}

void dir_list2(const char *path,
               const char *patern,
               int recursive,
               int only_dir,
               void(*on_item)(const char *filename)) {
    _dir_list2(path, patern, recursive, only_dir, on_item);
}


void dir_list_free(char **files,
                   uint32_t count,
                   struct ce_alloc_t0 *allocator) {
    for (uint32_t i = 0; i < count; ++i) {
        CE_FREE(allocator, files[i]);
    }

    CE_FREE(allocator, files);
}


int dir_make(const char *path) {
    struct stat st;
    const int mode = 0777;

    if (stat(path, &st) != 0) {
        if (mkdir(path, mode) != 0 && errno != EEXIST) {
            return 0;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return 0;
    }

    return 1;
}

int dir_make_path(const char *path) {
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
}

const char *path_filename(const char *path) {
    const char *ch = strrchr(path, DIR_DELIM_CH);
    return ch != NULL ? ch + 1 : path;
}

void path_basename(const char *path,
                   char *out) {
    const char *filename = path_filename(path);
    const char *ch = strchr(filename, '.');

    if (ch == NULL) {
        strcpy(out, filename);
        return;
    }

    const size_t basename_len = (ch - filename);
    memcpy(out, filename, basename_len);
    out[basename_len] = '\0';
}

void path_dir(char *out,
              const char *path) {
    const char *ch = strrchr(path, DIR_DELIM_CH);

    if (ch != NULL) {
        size_t len = ch - path;
        memcpy(out, path, len);
        out[len] = '\0';
    }
}

void path_dirname(char *out,
                  const char *path) {
    char buffer[128] = {};
    strcpy(buffer, path);

    const size_t path_len = strlen(path);

    if (buffer[path_len - 1] == DIR_DELIM_CH) {
        buffer[path_len - 1] = '\0';
    }

    char *ch = strrchr(buffer, DIR_DELIM_CH);

    if (ch != NULL) {
        strcpy(out, ch + 1);
    } else {
        strcpy(out, buffer);
    }
}

const char *path_extension(const char *path) {
    const char *filename = path_filename(path);
    const char *ch = strrchr(filename, '.');

    if (ch == NULL) {
        return NULL;
    }

    return ch + 1;
}

void path_join(char **buffer,
               struct ce_alloc_t0 *allocator,
               uint32_t count,
               ...) {

    va_list arguments;
    va_start (arguments, count);

    for (uint32_t i = 0; i < count; ++i) {
        const char *t = va_arg (arguments, const char*);

        if (!t) {
            continue;
        }

        if (ce_buffer_size(*buffer)) {
            if ((*buffer)[ce_buffer_size(*buffer) - 1] != DIR_DELIM_CH) {
                ce_buffer_push_ch(*buffer, DIR_DELIM_CH, allocator);
            }
        }

        ce_buffer_printf(buffer, allocator, "%s", t);
    }

    va_end (arguments);
}

void copy_file(ce_alloc_t0 *allocator,
               const char *from,
               const char *to) {
    ce_vio_t0 *source_vio = ce_os_vio_a0->from_file(from,
                                                    VIO_OPEN_READ);

    char *data = CE_ALLOC(allocator, char,
                          source_vio->vt->size(source_vio->inst));

    size_t size = (size_t) source_vio->vt->size(source_vio->inst);
    source_vio->vt->read(source_vio->inst, data, sizeof(char), size);
    ce_os_vio_a0->close(source_vio);

    ce_vio_t0 *build_vio = ce_os_vio_a0->from_file(to, VIO_OPEN_WRITE);

    build_vio->vt->write(build_vio->inst, data, sizeof(char), size);
    ce_os_vio_a0->close(build_vio);

    CE_FREE(allocator, data);
}

bool is_dir(const char *path) {
    struct stat sb;
    return (stat(path, &sb) == 0) && S_ISDIR(sb.st_mode);
}

struct ce_os_path_a0 path_api = {
        .list = dir_list,
        .list_free = dir_list_free,
        .make_path = dir_make_path,
        .filename = path_filename,
        .basename = path_basename,
        .dir = path_dir,
        .dirname = path_dirname,
        .extension = path_extension,
        .join = path_join,
        .file_mtime = file_mtime,
        .copy_file = copy_file,
        .is_dir = is_dir,
};

struct ce_os_path_a0 *ce_os_path_a0 = &path_api;
