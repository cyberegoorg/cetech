//! \defgroup Config
//! Config system
//! \{
#ifndef CETECH_CONFIG_H
#define CETECH_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

struct cel_alloc;

struct ct_coredb_object_t;

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
                               struct cel_alloc *alloc);

    struct ct_coredb_object_t* (*config_object)();

    //! Dump all variables to log
    void (*log_all)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_CONFIG_H
//! \}