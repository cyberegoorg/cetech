#ifndef CETECH_PROCESS_H
#define CETECH_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct process_api_v0 {
    int (*exec)(const char *argv);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_PROCESS_H
