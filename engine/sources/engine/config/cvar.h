#ifndef CETECH_CONFIG_H
#define CETECH_CONFIG_H

//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <celib/os/cmd_line.h>
#include "celib/types.h"

typedef struct {
    u64 idx;
} cvar_t;

enum cvar_type {
    CV_NONE = 0,
    CV_FLOAT,
    CV_INT,
    CV_STRING
};


//==============================================================================
// Interface
//==============================================================================

int cvar_init();

void cvar_shutdown();

struct ConfigApiV1 {
    void (*load_global)();

    void (*compile_global)();

    int (*parse_core_args)(struct args args);

    int (*parse_args)(struct args args);

    cvar_t (*find)(const char *name);

    cvar_t (*find_or_create)(const char *name,
                             int *is_new);

    cvar_t (*new_float)(const char *name,
                        const char *desc,
                        float f);

    cvar_t (*new_int)(const char *name,
                      const char *desc,
                      int i);

    cvar_t (*new_str)(const char *name,
                      const char *desc,
                      const char *s);

    float (*get_float)(cvar_t var);

    int (*get_int)(cvar_t var);

    const char *(*get_string)(cvar_t var);

    enum cvar_type (*get_type)(cvar_t var);

    void (*set_float)(cvar_t var,
                      float f);

    void (*set_int)(cvar_t var,
                    int i);

    void (*set_string)(cvar_t var,
                       const char *s);

    void (*log_all)();
};

#endif //CETECH_CONFIG_H
