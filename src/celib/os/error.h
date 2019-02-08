//                          **OS error**
//

#ifndef CE_OS_ERROR_H
#define CE_OS_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#define CE_OS_ERROR_API \
    CE_ID64_0("ce_os_error_a0", 0x67f389a4cfbbeb51ULL)


struct ce_os_error_a0 {
    // Assert
    void (*assert)(const char *where,
                   const char *condition,
                   const char *filename,
                   int line);

    char *(*stacktrace)(int skip);

    void (*stacktrace_free)(char *st);
};


CE_MODULE(ce_os_error_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_OS_ERROR_H
