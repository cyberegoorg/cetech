#include <cetech/kernel/api_system.h>
#include <cetech/kernel/module.h>


#include <cetech/kernel/os.h>
#include <cetech/kernel/log.h>

CETECH_DECL_API(log_api_v0);

#include "path_linux.h"
#include "vio_linux.h"
#include "sdl2_object.h"
#include "sdl2_process.h"


static struct os_vio_api_v0 vio_api = {
        .from_file = vio_from_file,
        .close = vio_close,
        .seek = vio_seek,
        .seek_to_end = vio_seek_to_end,
        .skip = vio_skip,
        .position = vio_position,
        .size = vio_size,
        .read = vio_read,
        .write = vio_write
};

static struct os_path_v0 path_api = {
        .list = dir_list,
        .list_free = dir_list_free,
        .make_path = dir_make_path,
        .filename = path_filename,
        .basename = path_basename,
        .dir = path_dir,
        .extension = path_extension,
        .join = path_join,
        .file_mtime = file_mtime
};

static struct os_process_api_v0 process_api = {
        .exec = exec
};

static struct os_object_api_v0 object_api = {
        .load  = load_object,
        .unload  = unload_object,
        .load_function  = load_function
};


extern void error_register_api(struct api_v0 *api);

namespace os_linux {
    void register_api(struct api_v0 *api) {
        CETECH_GET_API(api, log_api_v0);

        api->register_api("os_path_v0", &path_api);
        api->register_api("os_vio_api_v0", &vio_api);
        api->register_api("os_process_api_v0", &process_api);
        api->register_api("os_object_api_v0", &object_api);
        error_register_api(api);
    }
}
