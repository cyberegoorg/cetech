#ifndef CETECH_CONFIG_H
#define CETECH_CONFIG_H


//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <corelib/module.inl>

struct ct_alloc;


//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Enums
//==============================================================================

//==============================================================================
// Api
//==============================================================================

//! Config API V0
struct ct_config_a0 {
    //! Parse commandline arguments.
    //! \param args Arguments
    //! \return 1 if ok else 0
    int (*parse_args)(int argc,
                      const char **argv);

    //! Load config from yaml file.
    //! \param yaml Yaml file path.
    //! \param alloc Allocator.
    int (*load_from_yaml_file)(const char *yaml,
                               struct ct_alloc *alloc);

    uint64_t (*obj)();

    //! Dump all variables to log
    void (*log_all)();
};

CT_MODULE(ct_config_a0);

#endif //CETECH_CONFIG_H
