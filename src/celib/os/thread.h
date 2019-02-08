//                          **OS thread**
//

#ifndef CE_OS_THREAD_H
#define CE_OS_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#define CE_OS_THREAD_API \
    CE_ID64_0("ce_os_thread_a0", 0x487c28f7f512ba0dULL)

typedef struct ce_thread_t0 {
    uint64_t o;
} ce_thread_t0;

typedef int (ce_thread_fce_t0)(void *data);

typedef struct ce_spinlock_t0 {
    uint64_t o;
} ce_spinlock_t0;

struct ce_os_thread_a0 {
    // Create new thread
    // - fce Thread fce
    // - name Thread name
    // - data Thread data
    ce_thread_t0 (*create)(ce_thread_fce_t0 fce,
                           const char *name,
                           void *data);

    // Kill thread
    // - thread thread
    void (*kill)(ce_thread_t0 thread);

    // Wait for thread
    // - thread Thread
    // - status Thread exit status
    void (*wait)(ce_thread_t0 thread,
                 int *status);

    // Get id for thread
    // - thread Thread
    uint64_t (*get_id)(ce_thread_t0 thread);

    // Get actual thread id
    uint64_t (*actual_id)();

    void (*yield)();

    void (*spin_lock)(ce_spinlock_t0 *lock);

    void (*spin_unlock)(ce_spinlock_t0 *lock);
};


CE_MODULE(ce_os_thread_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_OS_THREAD_H
