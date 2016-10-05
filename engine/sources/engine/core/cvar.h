#ifndef CETECH_CONFIG_API_H
#define CETECH_CONFIG_API_H

//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <celib/os/cmd_line.h>
#include "celib/types.h"
#include "types.h"

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

void cvar_register_resource();

void cvar_load_global();

void cvar_compile_global();

int cvar_parse_core_args(struct args args);

int cvar_parse_args(struct args args);

cvar_t cvar_find(const char *name);

cvar_t cvar_find_or_create(const char *name,
                           int *new);

cvar_t cvar_new_float(const char *name,
                      const char *desc,
                      float f);

cvar_t cvar_new_int(const char *name,
                    const char *desc,
                    int i);

cvar_t cvar_new_str(const char *name,
                    const char *desc,
                    const char *s);

float cvar_get_float(cvar_t var);

int cvar_get_int(cvar_t var);

const char *cvar_get_string(cvar_t var);

enum cvar_type cvar_get_type(cvar_t var);

void cvar_set_float(cvar_t var,
                    float f);

void cvar_set_int(cvar_t var,
                  int i);

void cvar_set_string(cvar_t var,
                     const char *s);

#endif //CETECH_CONFIG_API_H
