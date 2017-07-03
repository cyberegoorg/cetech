#ifndef CETECH__CONFIG_H
#define CETECH__CONFIG_H

#include <cetech/kernel/config.h>

namespace config {
    int init(struct api_v0 *api);

    void shutdown();

    void compile_global(struct app_api_v0 *app_api);
    void load_global(struct app_api_v0 *app_api);

    int parse_args(int argc,
                   const char **argv);

    cvar_t find(const char *name);

    cvar_t find_or_create(const char *name,
                          int *new_);

    cvar_t new_float(const char *name,
                     const char *desc,
                     float f);

    cvar_t new_int(const char *name,
                   const char *desc,
                   int i);

    cvar_t new_str(const char *name,
                   const char *desc,
                   const char *s);

    float get_float(cvar_t var);

    int get_int(cvar_t var);

    const char *get_string(cvar_t var);

    enum cvar_type get_type(cvar_t var);

    void set_float(cvar_t var,
                   float f);

    void set_int(cvar_t var,
                 int i);

    void set_string(cvar_t var,
                    const char *s);

    void log_all();
}

#endif //CETECH__CONFIG_H
