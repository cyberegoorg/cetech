#ifndef CETECH_CONFIG_H
#define CETECH_CONFIG_H

//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <celib/os/cmd_line.h>
#include "celib/types.h"
#include "types.h"
#include "config_api.h"

//==============================================================================
// Interface
//==============================================================================

int cvar_init();

void cvar_shutdown();

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

void cvar_log_all();


static const struct ConfigApiV1 ConfigApiV1 = {
        .load_global = cvar_load_global,
        .compile_global = cvar_compile_global,
        .parse_core_args = cvar_parse_core_args,
        .parse_args = cvar_parse_args,
        .find = cvar_find,
        .find_or_create = cvar_find_or_create,
        .new_float = cvar_new_float,
        .new_int = cvar_new_int,
        .new_str = cvar_new_str,
        .get_float = cvar_get_float,
        .get_int = cvar_get_int,
        .get_string = cvar_get_string,
        .get_type  = cvar_get_type,
        .set_float = cvar_set_float,
        .set_int = cvar_set_int,
        .set_string = cvar_set_string,
        .log_all = cvar_log_all,
};

#endif //CETECH_CONFIG_H
