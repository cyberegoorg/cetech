#include <cetech/kernel/application.h>
#include <cetech/kernel/config.h>

#include <cetech/core/module.h>
#include <cetech/core/thread.h>
#include <cetech/core/window.h>
#include <cetech/core/os.h>

#include "sdl_cpu.h"
#include "sdl_window.h"
#include "sdl_thread.h"
#include "sdl2_object.h"
#include "sdl_time.h"

void *sdl_get_module_api(int api) {
    switch (api) {
        case PLUGIN_EXPORT_API_ID: {
            static struct module_api_v0 module = {0};
            return &module;
        }


        case OS_THREAD_API_ID: {
            static struct thread_api_v0 api = {
                    .create = thread_create,
                    .kill = thread_kill,
                    .wait = thread_wait,
                    .get_id = thread_get_id,
                    .actual_id = thread_actual_id,
                    .yield = thread_yield,
                    .spin_lock = thread_spin_lock,
                    .spin_unlock = thread_spin_unlock
            };
            return &api;
        }

        case WINDOW_API_ID: {
            static struct window_api_v0 api = {
                    .create = window_new,
                    .create_from = window_new_from,
                    .destroy = window_destroy,
                    .set_title = window_set_title,
                    .get_title = window_get_title,
                    .update = window_update,
                    .resize = window_resize,
                    .get_size = window_get_size,
                    .native_window_ptr = window_native_window_ptr,
                    .native_display_ptr = window_native_display_ptr
            };
            return &api;
        }

        case CPU_API_ID: {
            static struct cpu_api_v0 api = {
                    .count = cpu_count
            };
            return &api;
        }


        case OBJECT_API_ID: {
            static struct object_api_v0 api = {
                    .load  = load_object,
                    .unload  = unload_object,
                    .load_function  = load_function
            };
            return &api;
        }

        case TIME_API_ID: {
            static struct time_api_v0 api = {
                    .get_ticks =get_ticks,
                    .get_perf_counter =get_perf_counter,
                    .get_perf_freq =get_perf_freq
            };
            return &api;
        }

        default:
            return NULL;
    }
}