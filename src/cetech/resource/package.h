#ifndef CETECH_PACKAGE_H
#define CETECH_PACKAGE_H

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <corelib/module.inl>


#define PACKAGE_TYPE \
    CT_ID64_0("package", 0xad9c6d9ed1e5e77aULL)

#define PACKAGE_TYPES_PROP \
    CT_ID64_0("types", 0xe6e09632cf72fc79ULL)


struct ct_task_counter_t;

//==============================================================================
// Api
//==============================================================================

struct ct_package_a0 {
    struct ct_task_counter_t *(*load)(uint64_t name);

    void (*unload)(uint64_t name);

    int (*is_loaded)(uint64_t name);

    void (*flush)(struct ct_task_counter_t *counter);
};

CT_MODULE(ct_package_a0);

#endif //CETECH_PACKAGE_H
