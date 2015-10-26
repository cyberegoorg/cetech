#include <cstring>

#include "cetech/filesystem/file.h"
#include "cetech/filesystem/filesystem.h"

#include "celib/string/stringid.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/memory/memory.h"
#include "celib/macros.h"

#include "cetech/platform/dir.h"
#include "cetech/platform/file.h"


namespace cetech {
    class DiskFile : public FSFile {
        public:
            File _file;

            DiskFile(const char* path, FSFile::OpenMode mode) {
                _file = file::from_file(path, mode == FSFile::WRITE ? "w" : "r");
            }

            virtual ~DiskFile() {
                file::close(_file);
            }

            virtual bool is_valid() {
                return !file::is_null(_file);
            }

            virtual void seek(size_t position) final {
                file::seek(_file, position, file::SW_SEEK_SET);
            };

            virtual void seek_to_end() final {
                file::seek(_file, 0, file::SW_SEEK_END);
            };

            virtual void skip(size_t bytes) final {
                file::seek(_file, bytes, file::SW_SEEK_CUR);
            };

            virtual void read(void* buffer, size_t size) final {
                file::read(_file, buffer, sizeof(char), size);
            };

            virtual void write(const void* buffer, size_t size)  final {
                file::write(_file, buffer, sizeof(char), size);
            };

            virtual void flush() final {};

            virtual bool end_of_file()  final {
                return position() == size();
            };

            virtual size_t size()  final {
                const size_t curent_pos = position();
                seek_to_end();
                const size_t sz = position();
                seek(curent_pos);

                return sz;
            };

            virtual size_t position() final {
                return file::tell(_file);
            };
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
            return file::mtime(abs_path);
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