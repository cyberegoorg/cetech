#include <cetech/module/module.h>
#include <cetech/os/thread.h>

#include "sdl_thread.h"

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


        default:
            return NULL;
    }
}