#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "common/macros.h"
#include "common/asserts.h"
#include "common/container/array.inl.h"

namespace cetech {
    namespace dir {
        bool mkdir(const char* path) {
            struct stat st;
            int mode = 0775;

            if (stat(path, &st) != 0) {
                if (::mkdir(path, mode) != 0 && errno != EEXIST) {
                    return false;
                }
            } else if (!S_ISDIR(st.st_mode)) {
                errno = ENOTDIR;
                return false;
            }

            return true;
        }

        bool mkpath(const char* path) {
            char* pp;
            char* sp;
            bool status = true;
            char* copypath = strdup(path);

            pp = copypath;
            while (status == true && (sp = strchr(pp, '/')) != 0) {
                if (sp != pp) {
                    *sp = '\0';
                    status = mkdir(copypath);
                    *sp = '/';
                }

                pp = sp + 1;
            }

            if (status == true) {
                status = mkdir(path);
            }

            free(copypath);
            return status;
        }

        void listdir(const char* name, Array < char* >& files) {
            DIR* dir;
            struct dirent* entry;

            if (!(dir = opendir(name))) {
                return;
            }

            if (!(entry = readdir(dir))) {
                return;
            }

            do {
                if (entry->d_type == 4) {
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                        continue;
                    }

                    char path[1024];
                    int len = 0;

                    if (name[strlen(name) - 1] != '/') {
                        len = snprintf(path, sizeof(path) - 1, "%s/%s/", name, entry->d_name);
                    } else {
                        len = snprintf(path, sizeof(path) - 1, "%s%s/", name, entry->d_name);
                    }

                    path[len] = '\0';

                    listdir(path, files);
                } else {
                    uint32_t size = strlen(name) + strlen(entry->d_name) + 2;
                    char* path = (char*)malloc(sizeof(char) * size);

                    snprintf(path, size - 1, "%s%s", name, entry->d_name);

                    array::push_back(files, path);
                }
            } while ((entry = readdir(dir)));

            closedir(dir);
        }

        void listdir_free(Array < char* >& files) {
            for (uint32_t i = 0; i < array::size(files); ++i) {
                free(files[i]);
            }
        }
    }
}

