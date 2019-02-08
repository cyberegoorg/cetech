//                          **OS process**
//

#ifndef CE_OS_PROCESS_H
#define CE_OS_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#define CE_OS_PROCESS_API \
    CE_ID64_0("ce_os_process_a0", 0xde6406a21bedc619ULL)

struct ce_os_process_a0 {
    int (*exec)(const char *argv);
};


CE_MODULE(ce_os_process_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_OS_PROCESS_H
