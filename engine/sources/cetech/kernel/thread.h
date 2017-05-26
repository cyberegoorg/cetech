#ifndef CETECH_THREAD_H
#define CETECH_THREAD_H

#include <stdint.h>


typedef int (*thread_fce_t)(void *data);

typedef struct {
    void *t;
} thread_t;

typedef struct {
    int lock;
} spinlock_t;

struct thread_api_v0 {
    //! Create new thread
    //! \param fce Thread fce
    //! \param name Thread name
    //! \param data Thread data
    //! \return new thread
    thread_t (*create)(thread_fce_t fce,
                       const char *name,
                       void *data);

    //! Kill thread
    //! \param thread thread
    void (*kill)(thread_t thread);

    //! Wait for thread
    //! \param thread Thread
    //! \param status Thread exit status
    void (*wait)(thread_t thread,
                 int *status);

    //! Get id for thread
    //! \param thread Thread
    //! \return ID
    uint64_t (*get_id)(thread_t thread);

    //! Get actual thread id
    //! \return Thread id
    uint64_t (*actual_id)();

    void (*yield)();

    void (*spin_lock)(spinlock_t *lock);

    void (*spin_unlock)(spinlock_t *lock);
};

#endif //CETECH_THREAD_H
