#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <memory.h>

#include <cetech/core/memory.h>
#include <cetech/core/errors.h>
#include <cetech/core/array.inl>
#include <cetech/core/string.h>
#include <cetech/core/fs.h>

#if defined(CETECH_LINUX)

#include <dirent.h>
#include <sys/stat.h>

#endif

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

//! List dir
//! \param path Dir path
//! \param recursive Resucrsive list?
//! \param files Result files
//! \param allocator Allocator
void dir_list(const char *path,
              int recursive,
              struct array_pchar *files,
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
            if (str_cmp(entry->d_name, ".") == 0 ||
                str_cmp(entry->d_name, "..") == 0) {
                continue;
            }

            char tmp_path[1024] = {0};
            int len = 0;

            if (path[str_len(path) - 1] != '/') {
                len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s/%s/", path,
                               entry->d_name);
            } else {
                len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s%s/", path,
                               entry->d_name);
            }

            dir_list(tmp_path, 1, files, allocator);
        } else {
            size_t size = str_len(path) + str_len(entry->d_name) + 3;
            char *new_path =
            CETECH_ALLOCATE(allocator, char,
                            sizeof(char) * size);

            if (path[str_len(path) - 1] != '/') {
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
void dir_list_free(struct array_pchar *files,
                   struct allocator *allocator) {
#if defined(CETECH_LINUX)
    for (int i = 0; i < ARRAY_SIZE(files); ++i) {
        CETECH_DEALLOCATE(allocator, ARRAY_AT(files, i));
    }
#endif
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

    memory_free(copypath);
    return status;
#endif
}
