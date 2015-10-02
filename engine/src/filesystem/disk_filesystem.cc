#include <cstring>

#include "filesystem/file.h"
#include "filesystem/filesystem.h"

#include "common/memory/memory.h"

#include "platforms/dir/dir.h"
#include "platforms/file/file.h"

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

    class DiskFileSystem : public FileSystem {
        public:
            char _root_path[1024];
            Allocator& _allocator;

            DiskFileSystem(const char* root_path, Allocator & allocator) : _root_path({0}), _allocator(allocator) {
                strcpy(_root_path, root_path);

                const size_t len = strlen(_root_path);
                if (_root_path[len - 1] != '/') {
                    _root_path[len] = '/';
                }
            }

            virtual FSFile* open(const char* path, FSFile::OpenMode mode) final {
                char abs_path[2048] = {0};
                absolute_path(abs_path, path);

                return MAKE_NEW(_allocator, DiskFile, abs_path, mode);
            }

            virtual void close(FSFile* file) final {
                MAKE_DELETE(_allocator, FSFile, file);
            };

            virtual bool exists(const char* path) final {/*TODO:*/
            };

            virtual bool is_directory(const char* path) final {/*TODO:*/
            };

            virtual bool is_file(const char* path)  final {/*TODO:*/
            };

            virtual void create_directory(const char* path)  final {
                dir::mkpath(path);
            };

            virtual void delete_directory(const char* path) final {/*TODO:*/
            };

            virtual void list_directory ( const char* path, cetech::Array < char* >& files ) final {
                dir::listdir(path, files);
            }

            virtual void create_file(const char* path)  final {/*TODO:*/
            };

            virtual void delete_file(const char* path)  final {/*TODO:*/
            };

            void absolute_path(char* buffer, const char* path) {
                std::sprintf(buffer, "%s%s", _root_path, path);
            }
    };

    namespace disk_filesystem {
        FileSystem* make(Allocator& alocator, const char* root_path) {
            return MAKE_NEW(alocator, DiskFileSystem, root_path, alocator);
        }

        void destroy(Allocator& alocator, FileSystem* fs) {
            MAKE_DELETE(alocator, FileSystem, fs);
        }
    }
}