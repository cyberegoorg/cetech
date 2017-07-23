#ifndef CETECH__LOG_SYSTEM_H
#define CETECH__LOG_SYSTEM_H

#include <cetech/log.h>

namespace logsystem {
    void init();

    void shutdown();

//    int logdb_init_db(const char *log_dir,
//                      struct ct_api_a0 *api);

    //void set_wid_clb(ct_log_get_wid_clb_t get_wid_clb);
}

#endif //CETECH__LOG_SYSTEM_H
