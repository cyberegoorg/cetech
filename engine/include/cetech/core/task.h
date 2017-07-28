//! \defgroup Task
//! Task system
//! \{

#ifndef CETECH_TASKMANAGER_H
#define CETECH_TASKMANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdatomic.h>

//==============================================================================
// Enums
//==============================================================================

//! Worker enum
enum ct_workers {
    TASK_WORKER_MAIN = 0, //!< Main worker
    TASK_WORKER1 = 1,     //!< Worker 1
    TASK_WORKER2 = 2,     //!< Worker 2
    TASK_WORKER3 = 3,     //!< Worker 3
    TASK_WORKER4 = 4,     //!< Worker 4
    TASK_WORKER5 = 5,     //!< Worker 5
    TASK_WORKER6 = 6,     //!< Worker 6
    TASK_WORKER7 = 7,     //!< Worker 7
    TASK_MAX_WORKERS,     //!< Max workers
};

//! Task affinity enum
enum ct_task_affinity {
    TASK_AFFINITY_NONE = 0,     //!< No affinity
    TASK_AFFINITY_MAIN = 1,     //!< Main worker
    TASK_AFFINITY_WORKER1 = 2,  //!< Worker 1
    TASK_AFFINITY_WORKER2 = 3,  //!< Worker 2
    TASK_AFFINITY_WORKER3 = 4,  //!< Worker 3
    TASK_AFFINITY_WORKER4 = 5,  //!< Worker 4
    TASK_AFFINITY_WORKER5 = 6,  //!< Worker 5
    TASK_AFFINITY_WORKER6 = 7,  //!< Worker 6
    TASK_AFFINITY_WORKER7 = 8,  //!< Worker 7
    TASK_AFFINITY_MAX
};

//CETECH_STATIC_ASSERT(TASK_AFFINITY_MAX == TASK_MAX_WORKERS + 1);

//==============================================================================
// Structs
//==============================================================================

//! Task item struct
struct ct_task_item {
    const char *name;               //!< Task name
    void (*work)(void *data);       //!< Task work
    void *data;                     //!< Worker data
    enum ct_task_affinity affinity; //!< Worker affinity
};

//==============================================================================
// Api
//==============================================================================

//! Task API V0
struct ct_task_a0 {
    //! Workers count
    //! \return Workers count
    int (*worker_count)();

    //! Add new task
    //! \param items Task item array
    //! \param count Task item count
    void (*add)(struct ct_task_item *items,
                uint32_t count);

    //! Do work for task system.
    //! \return 1 if make some work else 0
    int (*do_work)();

    //! Wait for signal
    //! \param signal Signal ptr
    //! \param value Signal value
    void (*wait_atomic)(atomic_int *signal,
                        int32_t value);

    //! Curent worker id
    //! \return Worker id
    char (*worker_id)();
};

#endif //CETECH_TASKMANAGER_H

#ifdef __cplusplus
}
#endif

//! \}