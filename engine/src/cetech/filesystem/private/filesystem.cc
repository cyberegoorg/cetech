#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

#include "celib/string/stringid.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/memory/memory.h"
#include "celib/macros.h"

#include "cetech/filesystem/file.h"
#include "cetech/filesystem/filesystem.h"
#include "cetech/log/log.h"


#if defined(CETECH_SDL2)
    #include "SDL2/SDL.h"
#endif


namespace cetech {
    class DiskFile : public FSFile {
        public:
#if defined(CETECH_SDL2)
            SDL_RWops * rwops;
#endif

            DiskFile(const char* path, FSFile::OpenMode mode) {
#if defined(CETECH_SDL2)
                rwops = SDL_RWFromFile(path, mode == FSFile::WRITE ? "w" : "r");
                CE_ASSERT_MSG( "FSFile", is_valid(), "%s", SDL_GetError(), SDL_GetError());
#endif
            }

            virtual ~DiskFile() {
                CE_ASSERT( "FSFile", is_valid());
#if defined(CETECH_SDL2)
                if (SDL_RWclose(rwops) != 0) {
                    log::error("FSFile", "close error: %s", SDL_GetError());
                }

#endif
            }

            virtual bool is_valid() {
#if defined(CETECH_SDL2)
                return rwops != nullptr;
#endif
            }

            virtual void seek(size_t position) final {
                CE_ASSERT( "FSFile", is_valid());

#if defined(CETECH_SDL2)
                SDL_RWseek(rwops, position, RW_SEEK_SET);
#endif
            };

            virtual void seek_to_end() final {
                CE_ASSERT( "FSFile", is_valid());
#if defined(CETECH_SDL2)
                SDL_RWseek(rwops, 0, RW_SEEK_END);
#endif
            };

            virtual void skip(size_t bytes) final {
                CE_ASSERT( "FSFile", is_valid());

#if defined(CETECH_SDL2)
                SDL_RWseek(rwops, bytes, RW_SEEK_CUR);
#endif
            };

            virtual size_t position() final {
                CE_ASSERT( "FSFile", is_valid());

#if defined(CETECH_SDL2)
                return SDL_RWtell(rwops);
#endif
            };

            virtual bool end_of_file()  final {
                CE_ASSERT( "FSFile", is_valid());

                return position() == size();
            };

            virtual size_t size()  final {
                CE_ASSERT( "FSFile", is_valid());

                const size_t curent_pos = position();
                seek_to_end();
                const size_t sz = position();
                seek(curent_pos);

                return sz;
            };

            virtual void read(void* buffer,
                              size_t size) final {
                CE_ASSERT( "FSFile", is_valid());
#if defined(CETECH_SDL2)
                SDL_RWread(rwops, buffer, sizeof(char), size);
#endif
            };

            virtual void write(const void* buffer,
                               size_t size)  final {
                CE_ASSERT( "FSFile", is_valid());

#if defined(CETECH_SDL2)
                SDL_RWwrite(rwops, buffer, sizeof(char), size);
#endif
            };

            virtual void flush() final {};
    };

    namespace {
        using namespace filesystem;

        struct FilesystemData {
            Allocator& _allocator;
            Hash < const char* > _dir_map;

            FilesystemData(Allocator & allocator) : _allocator(allocator), _dir_map(allocator) {}
        };

        struct Globals {
            static const int MEMORY = sizeof(FilesystemData);
            char buffer[MEMORY];

            FilesystemData* data;

            Globals() : buffer {
                0
            }, data(0) {}
        } _globals;

        void absolute_path(char* buffer,
                           StringId64_t root,
                           const char* path) {
            std::sprintf(buffer, "%s%s", root_dir(root), path);
        }

        bool mkdir(const char* path) {
            struct stat st;
            const int mode = 0775;

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

        void listdir(const char* name,
                     Array < char* >& files) {
            DIR* dir;
            struct dirent* entry;

            if (!(dir = opendir(name))) {
                return;
            }

            if (!(entry = readdir(dir))) {
                closedir(dir);
                return;
            }

            do {
                if (entry->d_type == 4) {
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                        continue;
                    }

                    char path[1024] = {0};
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
                    char* path = (char*)memory_globals::default_allocator().allocate(sizeof(char) * size);

                    snprintf(path, size - 1, "%s%s", name, entry->d_name);

                    array::push_back(files, path);
                }
            } while ((entry = readdir(dir)));

            closedir(dir);
        }

        void listdir_free(Array < char* >& files) {
            for (uint32_t i = 0; i < array::size(files); ++i) {
                memory_globals::default_allocator().deallocate(files[i]);
            }
        }
    }

    namespace filesystem {
        void map_root_dir(StringId64_t name,
                          const char* dir) {
            CE_ASSERT( "filesystem", dir[strlen(dir) - 1] == '/' );

            hash::set < const char* > (_globals.data->_dir_map, name, strdup(dir));
        }

        FSFile& open(StringId64_t root,
                     const char* path,
                     FSFile::OpenMode mode) {
            char abs_path[2048] = {0};
            absolute_path(abs_path, root, path);
            return *MAKE_NEW(_globals.data->_allocator, DiskFile, abs_path, mode);
        }

        void close(FSFile& file) {
            MAKE_DELETE(_globals.data->_allocator, FSFile, &file);
        };

        const char* root_dir(StringId64_t root) {
            return hash::get < const char* > (_globals.data->_dir_map, root, nullptr);
        };

        void create_directory(StringId64_t root,
                              const char* path) {
            char buffer[4096] = {0};
            absolute_path(buffer, root, path ? path : "");
            mkpath(buffer);
        };

        void list_directory (StringId64_t root,
                             const char* path,
                             cetech::Array < char* >& files ) {
            char buffer[4096] = {0};
            absolute_path(buffer, root, path ? path : "");
            listdir(buffer, files);
        }

        void free_list_directory(Array < char* >& files) {
            listdir_free(files);
        };

        time_t file_mtime(StringId64_t root,
                          const char* path) {
            char abs_path[4096] = {0};
            absolute_path(abs_path, root, path);

            struct stat st;
            stat(abs_path, &st);

            return st.st_mtime;
        }

    };

    namespace filesystem_globals {
        void init() {
            log::info("filesystem_globals", "Init");

            char* p = _globals.buffer;
            _globals.data = new(p) FilesystemData(memory_globals::default_allocator());
        }

        void shutdown() {
            log::info("filesystem_globals", "Shutdown");

            _globals.data->~FilesystemData();
            _globals = Globals();
        }
    }

}
