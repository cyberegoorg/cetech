//                          **OS input**
//

#ifndef CE_OS_INPUT_H
#define CE_OS_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#define CE_OS_INPUT_API \
    CE_ID64_0("ce_os_input_a0", 0x5b789087d6f8fcd7ULL)

struct ce_os_input_a0 {
    const char *(*get_clipboard_text)();

    void (*set_clipboard_text)(const char *text);
};


CE_MODULE(ce_os_input_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_OS_INPUT_H
