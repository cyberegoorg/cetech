#ifndef CETECH_CONFIG_API_H
#define CETECH_CONFIG_API_H

//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

//==============================================================================
// Typedefs
//==============================================================================

typedef size_t config_var_t;


//==============================================================================
// Interface
//==============================================================================

void config_init();

void config_shutdown();

config_var_t config_find_or_create(const char *, int *);

config_var_t config_new_float(const char *, const char *, float);

config_var_t config_new_int(const char *, const char *, int);

config_var_t config_new_string(const char *, const char *, const char *);

float config_get_float(config_var_t);

int config_get_int(config_var_t);

const char *config_get_string(config_var_t);

void config_set_float(config_var_t, float);

void config_set_int(config_var_t, int);

void config_set_string(config_var_t, const char *);

#endif //CETECH_CONFIG_API_H
