#include <cetech/module.h>
#include <cetech/thread.h>
#include <cetech/window.h>
#include <cetech/object.h>
#include <cetech/time.h>

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
                    .create = cel_thread_create,
                    .kill = cel_thread_kill,
                    .wait = cel_thread_wait,
                    .get_id = cel_thread_get_id,
                    .actual_id = cel_thread_actual_id,
                    .yield = cel_thread_yield,
                    .spin_lock = cel_thread_spin_lock,
                    .spin_unlock = cel_thread_spin_unlock
            };
            return &api;
        }

        case WINDOW_API_ID: {
            static struct window_api_v0 api = {
                    .create = cel_window_new,
                    .create_from = cel_window_new_from,
                    .destroy = cel_window_destroy,
                    .set_title = cel_window_set_title,
                    .get_title = cel_window_get_title,
                    .update = cel_window_update,
                    .resize = cel_window_resize,
                    .get_size = cel_window_get_size,
                    .native_window_ptr = cel_window_native_cel_window_ptr,
                    .native_display_ptr = cel_window_native_display_ptr
            };
            return &api;
        }

        case CPU_API_ID: {
            static struct cpu_api_v0 api = {
                    .count = cel_cpu_count
            };
            return &api;
        }


        case OBJECT_API_ID: {
            static struct object_api_v0 api = {
                    .load  = cel_load_object,
                    .unload  = cel_unload_object,
                    .load_function  = cel_load_function
            };
            return &api;
        }

        case TIME_API_ID: {
            static struct time_api_v0 api = {
                    .get_ticks =cel_get_ticks,
                    .get_perf_counter =cel_get_perf_counter,
                    .get_perf_freq =cel_get_perf_freq
            };
            return &api;
        }

        default:
            return NULL;
    }
}