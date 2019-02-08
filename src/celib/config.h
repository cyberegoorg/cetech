#ifndef CE_CONFIG_H
#define CE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib_types.h"

#define CE_CONFIG_API \
    CE_ID64_0("ce_config_a0", 0xd75a5088e70ad1bbULL)

struct ce_alloc_t0;

struct ce_config_a0 {
    //! Parse commandline arguments.
    int (*parse_args)(int argc,
                      const char **argv);

    //! Load config from yaml file.
    int (*load_from_yaml_file)(const char *yaml,
                               struct ce_alloc_t0 *alloc);

    uint64_t (*obj)();

    //! Dump all variables to log
    void (*log_all)();
};

CE_MODULE(ce_config_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_CONFIG_H
