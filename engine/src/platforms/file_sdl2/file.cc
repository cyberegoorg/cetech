#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "common/macros.h"
#include "common/log/log.h"
#include "common/asserts.h"
#include "common/container/array.inl.h"
#include "common/crypto/murmur_hash.inl.h"
#include "common/math/vector2.inl.h"
#include "application.h"

#include "platforms/file/file.h"
#include "platforms/file_sdl2/file_types.h"

#include "SDL2/SDL.h"

namespace cetech {
        namespace file {
	  
        void init() {
        }

        void shutdown() {
        }

        void frame_start() {
        }

        void frame_end() {
        }

            File from_file(const char* path, const char* mode) {
                SDL_RWops* rwops = SDL_RWFromFile(path, mode);

                if (!rwops) {
                    log::warning("sys", "Open file error: %s", path, SDL_GetError());
                }

                return (struct File) {
                           rwops
                };
            }

            bool is_null(const File& f) {
                return f.ops == nullptr;
            }

            int close(const File& f) {
                return SDL_RWclose(f.ops);
            }

            size_t read(const File& f, void* ptr, size_t size, size_t maxnum) {
                return SDL_RWread(f.ops, ptr, size, maxnum);
            }

            size_t write(const File& f, const void* ptr, size_t size, size_t num) {
                return SDL_RWwrite(f.ops, ptr, size, num);
            }

            int64_t seek(const File& f, int64_t offset, SeekWhence whence) {
                int wh = 0;

                switch (whence) {
                case SW_SEEK_SET:
                    wh = RW_SEEK_SET;
                    break;

                case SW_SEEK_CUR:
                    wh = RW_SEEK_CUR;
                    break;

                case SW_SEEK_END:
                    wh = RW_SEEK_END;
                    break;
                }

                return SDL_RWseek(f.ops, offset, wh);
            }

            int64_t tell(const File& f) {
                return SDL_RWtell(f.ops);
            }


            size_t size(const File& f) {
                size_t size;

                seek(f, 0, SW_SEEK_END);

                size = tell(f);

                seek(f, 0, SW_SEEK_SET);

                return size;
            }
        }
}

