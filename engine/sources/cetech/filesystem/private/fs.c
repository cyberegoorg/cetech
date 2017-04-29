#include <time.h>

#include "memory.h"

#include "cetech/os/errors.h"
#include "cetech/containers/array.inl"
#include "cetech/filesystem/fs.h"

#if defined(CELIB_LINUX)

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>

#endif

//==============================================================================
// File Interface
//==============================================================================

//! Get file modified time
//! \param path File path
//! \return Modified time
uint32_t cel_file_mtime(const char *path) {
#if defined(CELIB_LINUX)
    struct stat st;
    stat(path, &st);
    return st.st_mtime;
#endif
}

//==============================================================================
// Path Interface
//==============================================================================

//! List dir
//! \param path Dir path
//! \param recursive Resucrsive list?
//! \param files Result files
//! \param allocator Allocator
void cel_dir_list(const char *path,
                  int recursive,
                  struct array_pchar *files,
                  struct cel_allocator *allocator) {
#if defined(CELIB_LINUX)
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

            cel_dir_list(tmp_path, 1, files, allocator);
        } else {
            size_t size = strlen(path) + strlen(entry->d_name) + 3;
            char *new_path = CEL_ALLOCATE(allocator, char, sizeof(char) * size);

            if (path[strlen(path) - 1] != '/') {
                snprintf(new_path, size - 1, "%s/%s", path, entry->d_name);
            } else {
                snprintf(new_path, size - 1, "%s%s", path, entry->d_name);
            }

            ARRAY_PUSH_BACK(pchar, files, new_path);
        }
    } while ((entry = readdir(dir)));

    closedir(dir);
#endif
}

//! Free list dir array
//! \param files Files array
//! \param allocator Allocator
void cel_dir_list_free(struct array_pchar *files,
                       struct cel_allocator *allocator) {
#if defined(CELIB_LINUX)
    for (int i = 0; i < ARRAY_SIZE(files); ++i) {
        CEL_DEALLOCATE(allocator, ARRAY_AT(files, i));
    }
#endif
}


//! Create dir
//! \param path Dir path
//! \return 1 of ok else 0
int cel_dir_make(const char *path) {
#if defined(CELIB_LINUX)
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
int cel_dir_make_path(const char *path) {
#if defined(CELIB_LINUX)
    char *pp;
    char *sp;
    int status = 1;
    char *copypath = strdup(path);

    pp = copypath;
    while (status == 1 && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            *sp = '\0';
            status = cel_dir_make(copypath);
            *sp = '/';
        }

        pp = sp + 1;
    }

    if (status == 1) {
        status = cel_dir_make(path);
    }

    free(copypath);
    return status;
#endif
}
