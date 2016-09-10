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
} config_var_t;


//==============================================================================
// Interface
//==============================================================================

int config_init();

void config_shutdown();

config_var_t config_find(const char *name);

config_var_t config_find_or_create(const char *name,
                                   int *new);

config_var_t config_new_float(const char *name,
                              const char *desc,
                              float f);

config_var_t config_new_int(const char *name,
                            const char *desc,
                            int i);

config_var_t config_new_string(const char *name,
                               const char *desc,
                               const char *s);

float config_get_float(config_var_t var);

int config_get_int(config_var_t var);

const char *config_get_string(config_var_t var);

void config_set_float(config_var_t var,
                      float f);

void config_set_int(config_var_t var,
                    int i);

void config_set_string(config_var_t var,
                       const char *s);

#endif //CETECH_CONFIG_API_H
