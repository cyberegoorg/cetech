#ifndef CETECH_THREAD_TYPES_H
#define CETECH_THREAD_TYPES_H

typedef int (*thread_fce_t)(void *data);

typedef struct {
    void *t;
} thread_t;

typedef struct {
    int lock;
} spinlock_t;

#endif //CETECH_THREAD_TYPES_H
