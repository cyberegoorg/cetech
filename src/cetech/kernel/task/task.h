//! \defgroup Task
//! Task system
//! \{

#ifndef CETECH_TASKMANAGER_H
#define CETECH_TASKMANAGER_H



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


//==============================================================================
// Structs
//==============================================================================

//! Task item struct
struct ct_task_item {
    const char *name;               //!< Task name
    void (*work)(void *data);       //!< Task work
    void *data;                     //!< Worker data
};

struct ct_task_counter_t;

//==============================================================================
// Api
//==============================================================================

//! Task API V0
struct ct_task_a0 {
    //! Workers count
    //! \return Workers count
    int (*worker_count)();

    //! Curent worker id
    //! \return Worker id
    char (*worker_id)();

    //! Add new task
    //! \param items Task item array
    //! \param count Task item count
    void (*add)(struct ct_task_item *items,
                uint32_t count, struct ct_task_counter_t** counter);

    void (*wait_for_counter)(struct ct_task_counter_t *signal, int32_t value);
};

#endif //CETECH_TASKMANAGER_H


//! \}