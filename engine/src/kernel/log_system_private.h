#ifndef CETECH__LOG_SYSTEM_H
#define CETECH__LOG_SYSTEM_H

#include <cetech/kernel/log.h>

namespace log {
    void init();

    void shutdown();

    void register_api(struct api_v0 *api);

    int logdb_init_db(const char *log_dir,
                      struct api_v0 *api);

    void set_wid_clb(log_get_wid_clb_t get_wid_clb);
}

#endif //CETECH__LOG_SYSTEM_H
