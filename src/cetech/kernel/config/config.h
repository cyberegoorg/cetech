#ifndef CETECH_CONFIG_H
#define CETECH_CONFIG_H


//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

struct ct_alloc;

struct ct_cdb_obj_t;

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

    struct ct_cdb_obj_t *(*config_object)();

    //! Dump all variables to log
    void (*log_all)();
};

#endif //CETECH_CONFIG_H
//! \}