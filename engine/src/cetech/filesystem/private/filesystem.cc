#include <cstring>
#include <sys/stat.h>


#include "cetech/filesystem/file.h"
#include "cetech/filesystem/filesystem.h"

#include "celib/string/stringid.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/memory/memory.h"
#include "celib/macros.h"

#include "cetech/platform/dir.h"

#include "cetech/log_system/log_system.h"


#if defined(CETECH_RUNTIME_SDL2)
    #include "SDL2/SDL.h"
#endif


namespace cetech {
    class DiskFile : public FSFile {
        public:
#if defined(CETECH_RUNTIME_SDL2)
            SDL_RWops * rwops;
#endif

            DiskFile(const char* path, FSFile::OpenMode mode) {
#if defined(CETECH_RUNTIME_SDL2)
                rwops = SDL_RWFromFile(path, mode == FSFile::WRITE ? "w" : "r");
                CE_ASSERT( is_valid() );
#endif
            }

            virtual ~DiskFile() {
                CE_ASSERT( is_valid() );

#if defined(CETECH_RUNTIME_SDL2)
                SDL_RWclose(rwops);
#endif
            }

            virtual bool is_valid() {
#if defined(CETECH_RUNTIME_SDL2)
                return rwops != nullptr;
#endif
            }

            virtual void seek(size_t position) final {
                CE_ASSERT( is_valid() );
                
#if defined(CETECH_RUNTIME_SDL2)
                SDL_RWseek(rwops, position, RW_SEEK_SET);
#endif
            };

            virtual void seek_to_end() final {
                CE_ASSERT( is_valid() );
#if defined(CETECH_RUNTIME_SDL2)
                SDL_RWseek(rwops, 0, RW_SEEK_END);
#endif
            };

            virtual void skip(size_t bytes) final {
                CE_ASSERT( is_valid() );
                
#if defined(CETECH_RUNTIME_SDL2)
                SDL_RWseek(rwops, bytes, RW_SEEK_CUR);
#endif
            };

            virtual size_t position() final {
                CE_ASSERT( is_valid() );
                
#if defined(CETECH_RUNTIME_SDL2)
                return SDL_RWtell(rwops);
#endif
            };

            virtual bool end_of_file()  final {
                CE_ASSERT( is_valid() );
                
                return position() == size();
            };

            virtual size_t size()  final {
                CE_ASSERT( is_valid() );
                
                const size_t curent_pos = position();
                seek_to_end();
                const size_t sz = position();
                seek(curent_pos);

                return sz;
            };
            
            virtual void read(void* buffer, size_t size) final {
                CE_ASSERT( is_valid() );
#if defined(CETECH_RUNTIME_SDL2)
                SDL_RWread(rwops, buffer, sizeof(char), size);
#endif
            };

            virtual void write(const void* buffer, size_t size)  final {
                CE_ASSERT( is_valid() );

#if defined(CETECH_RUNTIME_SDL2)
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

            Globals() : data(0) {}
        } _globals;

        void absolute_path(char* buffer, StringId64_t root, const char* path) {
            std::sprintf(buffer, "%s%s", root_dir(root), path);
        }

    }

    namespace filesystem {
        void map_root_dir(StringId64_t name, const char* dir) {
            CE_ASSERT( dir[strlen(dir) - 1] == '/' );

            hash::set < const char* > (_globals.data->_dir_map, name, strdup(dir));
        }

        FSFile& open(StringId64_t root, const char* path, FSFile::OpenMode mode) {
            char abs_path[2048] = {0};
            absolute_path(abs_path, root, path);
            return *MAKE_NEW(_globals.data->_allocator, DiskFile, abs_path, mode);
        }

        void close(FSFile& file) {
            MAKE_DELETE(_globals.data->_allocator, FSFile, &file);
        };

        bool exists(StringId64_t root, const char* path) {     /*TODO: #43*/
            CE_UNUSED(path);
            return false;
        };

        bool is_directory(StringId64_t root, const char* path) {     /*TODO: #43*/
            CE_UNUSED(path);
            return false;
        };

        bool is_file(StringId64_t root, const char* path) {      /*TODO: #43*/
            CE_UNUSED(path);
            return false;
        };

        const char* root_dir(StringId64_t root) {
            return hash::get < const char* > (_globals.data->_dir_map, root, nullptr);
        };

        void create_directory(StringId64_t root, const char* path) {
            dir::mkpath(path);
        };

        void delete_directory(StringId64_t root, const char* path) {     /*TODO: #43*/
            CE_UNUSED(path);
        };

        void list_directory (StringId64_t root, const char* path, cetech::Array < char* >& files ) {
            char buffer[4096] = {0};
            absolute_path(buffer, root, path ? path : "");
            dir::listdir(buffer, files);
        }

        void create_file(StringId64_t root, const char* path) {      /*TODO: #43*/
            CE_UNUSED(path);
        };

        void delete_file(StringId64_t root, const char* path) {      /*TODO: #43*/
            CE_UNUSED(path);
        };

        time_t file_mtime(StringId64_t root, const char* path) {
            char abs_path[4096] = {0};
            absolute_path(abs_path, root, path);

            struct stat st;
            stat(abs_path, &st);

            return st.st_mtime;
        }

    };

    namespace filesystem_globals {
        void init() {
            char* p = _globals.buffer;
            _globals.data = new(p) FilesystemData(memory_globals::default_allocator());
        }

        void shutdown() {
            _globals = Globals();
        }
    }

}