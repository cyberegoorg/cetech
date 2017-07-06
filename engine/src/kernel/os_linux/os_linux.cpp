#include <cetech/kernel/api_system.h>


#include <cetech/kernel/os.h>
#include <cetech/kernel/log.h>

CETECH_DECL_API(ct_log_a0);

#include "path_linux.h"
#include "object_linux.h"
#include "process_linux.h"

static ct_path_a0 path_api = {
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

static ct_process_a0 process_api = {
        .exec = exec
};

static ct_object_a0 object_api = {
        .load  = load_object,
        .unload  = unload_object,
        .load_function  = load_function
};


extern void error_register_api(ct_api_a0 *api);

namespace os {
    void register_api(ct_api_a0 *api) {
        CETECH_GET_API(api, ct_log_a0);

        api->register_api("ct_path_a0", &path_api);
        api->register_api("ct_process_a0", &process_api);
        api->register_api("ct_object_a0", &object_api);
        error_register_api(api);
    }
}
