#ifndef CETECH_CONFIG_API_H
#define CETECH_CONFIG_API_H

//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include "celib/types.h"

//==============================================================================
// Typedefs
//==============================================================================

typedef struct {
    u64 idx;
} cvar_t;


//==============================================================================
// Interface
//==============================================================================

int config_init();

void config_shutdown();

cvar_t config_find(const char *name);

cvar_t config_find_or_create(const char *name,
                             int *new);

cvar_t config_new_float(const char *name,
                        const char *desc,
                        float f);

cvar_t config_new_int(const char *name,
                      const char *desc,
                      int i);

cvar_t config_new_string(const char *name,
                         const char *desc,
                         const char *s);

float config_get_float(cvar_t var);

int config_get_int(cvar_t var);

const char *config_get_string(cvar_t var);

void config_set_float(cvar_t var,
                      float f);

void config_set_int(cvar_t var,
                    int i);

void config_set_string(cvar_t var,
                       const char *s);

#endif //CETECH_CONFIG_API_H
