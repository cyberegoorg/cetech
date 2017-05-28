#ifndef CETECH_PROCESS_H
#define CETECH_PROCESS_H

#include <stdint.h>

struct process_api_v0 {
    int (*exec)(const char *argv);
};

#endif //CETECH_PROCESS_H
