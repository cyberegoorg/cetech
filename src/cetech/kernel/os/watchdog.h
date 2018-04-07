#ifndef CETECH_CPU_H
#define CETECH_CPU_H

#include <cetech/kernel/memory/allocator.h>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Api
//==============================================================================

enum ct_watchdog_ev_type {
    CT_WATCHDOG_EVENT_NONE = 0,
    CT_WATCHDOG_EVENT_FILE_MODIFIED = 1
};

struct ct_watchdog_ev_header {
    uint32_t type;
    uint64_t size;
};

struct ct_wd_ev_file_write_end {
    ct_watchdog_ev_header header;
    const char *dir;
    char *filename;
};


typedef void ct_watchdog_instance_t;

//==============================================================================
// Api
//==============================================================================


struct ct_watchdog {
    ct_watchdog_instance_t *inst;

    void (*add_dir)(ct_watchdog_instance_t *inst,
                    const char *path,
                    bool recursive);

    void (*fetch_events)(ct_watchdog_instance_t *inst);

    ct_watchdog_ev_header *(*event_begin)(ct_watchdog_instance_t *inst);

    ct_watchdog_ev_header *(*event_end)(ct_watchdog_instance_t *inst);

    ct_watchdog_ev_header *(*event_next)(ct_watchdog_instance_t *inst,
                                         ct_watchdog_ev_header *header);
};

struct ct_watchdog_a0 {
    struct ct_watchdog *(*create)(struct ct_alloc *alloc);

    void (*destroy)(struct ct_watchdog *watchdog);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_CPU_Hs
