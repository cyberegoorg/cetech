//                          **OS object**
//

#ifndef CE_OS_OBJECT_H
#define CE_OS_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#define CE_OS_OBJECT_API \
    CE_ID64_0("ce_os_object_a0", 0x39f93df9f2df7e0fULL)


struct ce_os_object_a0 {
    // Load shared lib
    void *(*load)(const char *path);

    // Unload shared lib
    void (*unload)(void *so);

    // Load function from shared lib
    void *(*load_function)(void *so,
                           const char *name);
};


CE_MODULE(ce_os_object_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_OS_OBJECT_H
