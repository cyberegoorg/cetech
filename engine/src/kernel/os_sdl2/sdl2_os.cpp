#include <cetech/kernel/api_system.h>
#include <cetech/kernel/module.h>

#include <cetech/kernel/sdl2_os.h>

#include <cetech/kernel/log.h>


CETECH_DECL_API(log_api_v0);

#include "sdl2_path.h"
#include "sdl2_process.h"
#include "sdl2_vio_base.h"
#include "sdl2_cpu.h"
#include "sdl2_window.h"
#include "sdl2_thread.h"
#include "sdl2_object.h"
#include "sdl2_time.h"
#include "sdl2_vio.h"

namespace machine {
    void init(struct api_v0 *api);

    void shutdown();

    void register_api(struct api_v0 *api);
}

static struct os_thread_api_v0 thread_api = {
        .create = thread_create,
        .kill = thread_kill,
        .wait = thread_wait,
        .get_id = thread_get_id,
        .actual_id = thread_actual_id,
        .yield = thread_yield,
        .spin_lock = thread_spin_lock,
        .spin_unlock = thread_spin_unlock
};

static struct os_window_api_v0 window_api = {
        .create = window_new,
        .create_from = window_new_from,
        .destroy = window_destroy,
        .set_title = window_set_title,
        .get_title = window_get_title,
        .update = window_update,
        .resize = window_resize,
        .size = window_get_size,
        .native_window_ptr = window_native_window_ptr,
        .native_display_ptr = window_native_display_ptr
};

static struct os_cpu_api_v0 cpu_api = {
        .count = cpu_count
};

static struct os_object_api_v0 object_api = {
        .load  = load_object,
        .unload  = unload_object,
        .load_function  = load_function
};

static struct os_time_api_v0 time_api = {
        .ticks =get_ticks,
        .perf_counter =get_perf_counter,
        .perf_freq =get_perf_freq
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

static struct os_process_api_v0 process_api = {
        .exec = exec
};

namespace os {
    void register_api(struct api_v0 *api) {
        api->register_api("os_cpu_api_v0", &cpu_api);
        api->register_api("os_time_api_v0", &time_api);
        api->register_api("os_process_api_v0", &process_api);
        api->register_api("os_thread_api_v0", &thread_api);
        api->register_api("os_object_api_v0", &object_api);
        api->register_api("os_window_api_v0", &window_api);
        api->register_api("os_path_v0", &path_api);
        api->register_api("os_vio_api_v0", &vio_api);

        machine::register_api(api);
    }
}

static void _init(struct api_v0 *api) {
    CETECH_GET_API(api, log_api_v0);
}

extern "C" void os_load_module(struct api_v0 *api) {
    _init(api);
    machine::init(api);
}

extern "C" void os_unload_module(struct api_v0 *api) {
    machine::shutdown();
}


