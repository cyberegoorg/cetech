#ifndef CETECH__CONFIG_H
#define CETECH__CONFIG_H

#include <cetech/kernel/config.h>

namespace config {
    int init(struct ct_api_v0 *api);

    void shutdown();

    void compile_global(const char* platform);

    void load_global(const char* platform);

    int parse_args(int argc,
                   const char **argv);

    ct_cvar_t find(const char *name);

    ct_cvar_t find_or_create(const char *name,
                          int *new_);

    ct_cvar_t new_float(const char *name,
                     const char *desc,
                     float f);

    ct_cvar_t new_int(const char *name,
                   const char *desc,
                   int i);

    ct_cvar_t new_str(const char *name,
                   const char *desc,
                   const char *s);

    float get_float(ct_cvar_t var);

    int get_int(ct_cvar_t var);

    const char *get_string(ct_cvar_t var);

    enum cvar_type get_type(ct_cvar_t var);

    void set_float(ct_cvar_t var,
                   float f);

    void set_int(ct_cvar_t var,
                 int i);

    void set_string(ct_cvar_t var,
                    const char *s);

    void log_all();
}

#endif //CETECH__CONFIG_H
