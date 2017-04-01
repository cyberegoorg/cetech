//! \defgroup Task
//! Task system
//! \{

#ifndef CETECH_TASKMANAGER_H
#define CETECH_TASKMANAGER_H

//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <stdatomic.h>

#include "celib/thread/types.h"


//==============================================================================
// Defines
//==============================================================================

#define CEL_ASSERT_IS_WORKER(where, worker_id)                  \
    CEL_ASSERT_MSG(where,                                       \
                   TaskApiV0.worker_id() == worker_id,          \
                   "Current worker: %d", TaskApiV0.worker_id())


//==============================================================================
// Enums
//==============================================================================

//! Worker enum
enum workers {
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
enum task_affinity {
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

CEL_STATIC_ASSERT(TASK_AFFINITY_MAX == TASK_MAX_WORKERS + 1);


//==============================================================================
// Typedefs
//==============================================================================

//! Task work
typedef void (*task_work_t)(void *data);


//==============================================================================
// Structs
//==============================================================================

//! Task item struct
struct task_item {
    const char *name;            //!< Task name
    task_work_t work;            //!< Task work
    void *data;                  //!< Worker data
    enum task_affinity affinity; //!< Worker affinity
};

//==============================================================================
// Api
//==============================================================================

//! Task API V0
struct TaskApiV0 {
    //! Workers count
    //! \return Workers count
    int (*worker_count)();

    //! Add new task
    //! \param items Task item array
    //! \param count Task item count
    void (*add)(struct task_item *items,
                u32 count);

    //! Do work for task system.
    //! \return 1 if make some work else 0
    int (*do_work)();

    //! Wait for signal
    //! \param signal Signal ptr
    //! \param value Signal value
    void (*wait_atomic)(atomic_int *signal,
                        u32 value);

    //! Curent worker id
    //! \return Worker id
    char (*worker_id)();
};

#endif //CETECH_TASKMANAGER_H

//! \}