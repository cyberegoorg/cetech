#ifndef CETECH_THREAD_H
#define CETECH_THREAD_H


#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <stddef.h>

//==============================================================================
// Thread
//==============================================================================

typedef void ct_thread_t;

typedef int (*ct_thread_fce_t)(void *data);

struct ct_spinlock {
    int lock;
};

struct ct_thread_a0 {
    //! Create new thread
    //! \param fce Thread fce
    //! \param name Thread name
    //! \param data Thread data
    //! \return new thread
    ct_thread_t *(*create)(ct_thread_fce_t fce,
                           const char *name,
                           void *data);

    //! Kill thread
    //! \param thread thread
    void (*kill)(ct_thread_t *thread);

    //! Wait for thread
    //! \param thread Thread
    //! \param status Thread exit status
    void (*wait)(ct_thread_t *thread,
                 int *status);

    //! Get id for thread
    //! \param thread Thread
    //! \return ID
    uint64_t (*get_id)(ct_thread_t *thread);

    //! Get actual thread id
    //! \return Thread id
    uint64_t (*actual_id)();

    void (*yield)();

    void (*spin_lock)(struct ct_spinlock *lock);

    void (*spin_unlock)(struct ct_spinlock *lock);
};


#ifdef __cplusplus
}
#endif

#endif //CETECH_THREAD_H
