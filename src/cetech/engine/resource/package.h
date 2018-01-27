//! \defgroup Resource
//! Resource system
//! \{
#ifndef CETECH_PACKAGE_H
#define CETECH_PACKAGE_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

struct ct_task_counter_t;

//==============================================================================
// Api
//==============================================================================

//! Package API V!
struct ct_package_a0 {
    struct ct_task_counter_t* (*load)(uint64_t name);
    void (*unload)(uint64_t name);
    int (*is_loaded)(uint64_t name);
    void (*flush)(struct ct_task_counter_t *counter);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_PACKAGE_H
//! |}