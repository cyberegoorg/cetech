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

struct ct_api_a0;
struct ct_memory_a0;
struct ct_app_a0;

struct cel_alloc;

//==============================================================================
// Typedefs
//==============================================================================

//! Cvar type
struct ct_cvar {
    uint64_t idx; //!< Idx
};

//==============================================================================
// Enums
//==============================================================================

//! Cvar types enum
enum cvar_type {
    CV_NONE = 0, //!< Invalid type
    CV_FLOAT,    //!< Float type
    CV_INT,      //!< Int type
    CV_STRING    //!< String type
};

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

    //! Find cvar
    //! \param name Cvar name
    //! \return Cvar or null cvar if name not found.
    struct ct_cvar (*find)(const char *name);

    //! Find cvar if exist else create new
    //! \param name Cvar name
    //! \param is_new Write 1 if cvar is new
    //! \return Cvar
    struct ct_cvar (*find_or_create)(const char *name,
                                     int *is_new);

    //! Create new float cvar
    //! \param name Name
    //! \param desc Description
    //! \param f Value
    //! \return Cvar
    struct ct_cvar (*new_float)(const char *name,
                                const char *desc,
                                float f);

    //! Create new intcvar
    //! \param name Name
    //! \param desc Description
    //! \param i Value
    //! \return Cvar
    struct ct_cvar (*new_int)(const char *name,
                              const char *desc,
                              int i);

    //! Create new string cvar
    //! \param name Name
    //! \param desc Description
    //! \param s Value
    //! \return Cvar
    struct ct_cvar (*new_str)(const char *name,
                              const char *desc,
                              const char *s);

    //! Get float value
    //! \param var Cvar
    //! \return Float value
    float (*get_float)(struct ct_cvar var);

    //! Get int value
    //! \param var Cvar
    //! \return Int value
    int (*get_int)(struct ct_cvar var);

    //! Get string value
    //! \param var Cvar
    //! \return String value
    const char *(*get_string)(struct ct_cvar var);

    //! Get cvar type
    //! \param var Cvar
    //! \return Cvart type
    enum cvar_type (*get_type)(struct ct_cvar var);

    //! Set float value
    //! \param var Cvar
    //! \param f Value
    void (*set_float)(struct ct_cvar var,
                      float f);

    //! Set int value
    //! \param var Cvar
    //! \param i Value
    void (*set_int)(struct ct_cvar var,
                    int i);

    //! Set string value
    //! \param var Cvar
    //! \param s Value
    void (*set_string)(struct ct_cvar var,
                       const char *s);

    //! Dump all variables to log
    void (*log_all)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_CONFIG_H
//! \}