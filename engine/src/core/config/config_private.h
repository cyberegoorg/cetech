#ifndef CETECH__CONFIG_H
#define CETECH__CONFIG_H

#include <cetech/core/config.h>

namespace config {
    int init(struct ct_api_a0 *api);

    void shutdown();

    void compile_global(const char *platform);

    void load_global(const char *platform);

    int parse_args(int argc,
                   const char **argv);

    struct ct_cvar find(const char *name);

    struct ct_cvar find_or_create(const char *name,
                                  int *new_);

    struct ct_cvar new_float(const char *name,
                             const char *desc,
                             float f);

    struct ct_cvar new_int(const char *name,
                           const char *desc,
                           int i);

    struct ct_cvar new_str(const char *name,
                           const char *desc,
                           const char *s);

    float get_float(struct ct_cvar var);

    int get_int(struct ct_cvar var);

    const char *get_string(struct ct_cvar var);

    enum cvar_type get_type(struct ct_cvar var);

    void set_float(struct ct_cvar var,
                   float f);

    void set_int(struct ct_cvar var,
                 int i);

    void set_string(struct ct_cvar var,
                    const char *s);

    void log_all();
}

#endif //CETECH__CONFIG_H
