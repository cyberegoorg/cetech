#ifndef CE_TASKMANAGER_H
#define CE_TASKMANAGER_H

#define CE_TASK_API \
    CE_ID64_0("ce_task_a0", 0x4dbd12f32a50782eULL)

//! Worker enum
enum ce_workers {
    TASK_WORKER_MAIN = 0,  //!< Main worker
    TASK_MAX_WORKERS = 32, //!< Max workers
};

//! Task item struct
struct ce_task_item {
    const char *name;               //!< Task name
    void (*work)(void *data);       //!< Task work
    void *data;                     //!< Worker data
};

struct ce_task_counter_t;

//! Task API V0
struct ce_task_a0 {
    //! Workers count
    //! \return Workers count
    int (*worker_count)();

    //! Curent worker id
    //! \return Worker id
    char (*worker_id)();

    //! Add new task
    //! \param items Task item array
    //! \param count Task item count
    void (*add)(struct ce_task_item *items,
                uint32_t count,
                struct ce_task_counter_t **counter);

    void (*wait_for_counter)(struct ce_task_counter_t *signal,
                             int32_t value);
};

CE_MODULE(ce_task_a0);

#endif //CE_TASKMANAGER_H
