#include <cetech/kernel/api_system.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/os.h>
#include <cetech/kernel/log.h>

CETECH_DECL_API(ct_log_a0);

#include "cpu_sdl2.h"
#include "window_sdl2.h"
#include "thread_sdl2.h"
#include "sdl2_time.h"
#include "vio_sdl2.h"

namespace machine_sdl {
    void init(ct_api_a0 *api);
    void shutdown();
}

static ct_thread_a0 thread_api = {
        .create = thread_create,
        .kill = thread_kill,
        .wait = thread_wait,
        .get_id = thread_get_id,
        .actual_id = thread_actual_id,
        .yield = thread_yield,
        .spin_lock = thread_spin_lock,
        .spin_unlock = thread_spin_unlock
};

static ct_window_a0 window_api = {
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

static ct_cpu_a0 cpu_api = {
        .count = cpu_count
};



static ct_time_a0 time_api = {
        .ticks =get_ticks,
        .perf_counter =get_perf_counter,
        .perf_freq =get_perf_freq
};

static ct_vio_a0 vio_api = {
        .from_file = vio_from_file,
};


extern "C" void os_load_module(ct_api_a0 *api) {
    CETECH_GET_API(api, ct_log_a0);

    api->register_api("ct_cpu_a0", &cpu_api);
    api->register_api("ct_time_a0", &time_api);

    api->register_api("ct_thread_a0", &thread_api);
    api->register_api("ct_window_a0", &window_api);
    api->register_api("ct_vio_a0", &vio_api);


    machine_sdl::init(api);
}

extern "C" void os_unload_module(ct_api_a0 *api) {
    machine_sdl::shutdown();
}


