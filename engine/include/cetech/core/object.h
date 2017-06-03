#ifndef CETECH_OBJECT_H
#define CETECH_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct object_api_v0 {
    void *(*load)(const char *path);

    void (*unload)(void *so);

    void *(*load_function)(void *so,
                           const char *name);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_OBJECT_H
