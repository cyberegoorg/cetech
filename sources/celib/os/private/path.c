#include <string.h>

#include <celib/memory/memory.h>
#include <celib/string/string.h>
#include <celib/containers/array.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

const char *path_filename(const char *path) {
    char *ch = strrchr(path, '/');
    return ch != NULL ? ch + 1 : path;
}

void path_basename(const char *path, char *out, size_t size) {
    const char *filename = path_filename(path);
    const char *ch = strrchr(filename, '.');

    if (ch == NULL) {
        memory_copy(out, filename, str_lenght(filename));
        return;
    }

    const size_t basename_len = (ch - filename) / sizeof(char);
    memory_copy(out, filename, basename_len);
}

const char *path_extension(const char *path) {
    const char *filename = path_filename(path);
    const char *ch = strrchr(filename, '.');

    if (ch == NULL) {
        return NULL;
    }

    return ch + 1;
}

void path_listdir(const char *path,
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
            if (recursive && (entry->d_type == 4) ) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }

                char tmp_path[1024] = {0};
                int len = 0;

                if (path[strlen(path) - 1] != '/') {
                    len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s/%s/", path, entry->d_name);
                } else {
                    len = snprintf(tmp_path, sizeof(tmp_path) - 1, "%s%s/", path, entry->d_name);
                }

                tmp_path[len] = '\0';

                path_listdir(tmp_path, 1, files, allocator);
            } else {
                size_t size = strlen(path) + strlen(entry->d_name) + 2;
                char *new_path = CE_ALLOCATE(allocator, char, sizeof(char) * size);

                snprintf(new_path, size - 1, "%s%s", path, entry->d_name);

                ARRAY_PUSH_BACK(pchar, files, new_path);
            }
        } while ((entry = readdir(dir)));

        closedir(dir);
#endif
}

void path_listdir_free(ARRAY_T(pchar) *files, struct allocator *allocator) {
    for (int i = 0; i < ARRAY_SIZE(files); ++i) {
        CE_DEALLOCATE(allocator, ARRAY_AT(files, i));
    }
}

#define DIR_DELIM "/"

i64 path_join(char* result, u64 maxlen, const char* base_path, const char* path) {
    return snprintf(result, maxlen, "%s" DIR_DELIM "%s", base_path, path);
}

int path_mkdir(const char* path) {
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
}

int path_mkpath(const char* path) {
    char* pp;
    char* sp;
    int status = 1;
    char* copypath = strdup(path);

    pp = copypath;
    while (status == 1 && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            *sp = '\0';
            status = path_mkdir(copypath);
            *sp = '/';
        }

        pp = sp + 1;
    }

    if (status == 1) {
        status = path_mkdir(path);
    }

    free(copypath);
    return status;
}
