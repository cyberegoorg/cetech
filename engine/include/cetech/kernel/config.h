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

struct ct_api_v0;
struct ct_memory_api_v0;
struct ct_app_api_v0;

//==============================================================================
// Typedefs
//==============================================================================

//! Cvar type
typedef struct {
    uint64_t idx; //!< Idx
} ct_cvar_t;


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
struct ct_config_api_v0 {
    //! Parse commandline arguments.
    //! \param args Arguments
    //! \return 1 if ok else 0
    int (*parse_args)(int argc,
                      const char **argv);

    //! Find cvar
    //! \param name Cvar name
    //! \return Cvar or null cvar if name not found.
    ct_cvar_t (*find)(const char *name);

    //! Find cvar if exist else create new
    //! \param name Cvar name
    //! \param is_new Write 1 if cvar is new
    //! \return Cvar
    ct_cvar_t (*find_or_create)(const char *name,
                             int *is_new);

    //! Create new float cvar
    //! \param name Name
    //! \param desc Description
    //! \param f Value
    //! \return Cvar
    ct_cvar_t (*new_float)(const char *name,
                        const char *desc,
                        float f);

    //! Create new intcvar
    //! \param name Name
    //! \param desc Description
    //! \param i Value
    //! \return Cvar
    ct_cvar_t (*new_int)(const char *name,
                      const char *desc,
                      int i);

    //! Create new string cvar
    //! \param name Name
    //! \param desc Description
    //! \param s Value
    //! \return Cvar
    ct_cvar_t (*new_str)(const char *name,
                      const char *desc,
                      const char *s);

    //! Get float value
    //! \param var Cvar
    //! \return Float value
    float (*get_float)(ct_cvar_t var);

    //! Get int value
    //! \param var Cvar
    //! \return Int value
    int (*get_int)(ct_cvar_t var);

    //! Get string value
    //! \param var Cvar
    //! \return String value
    const char *(*get_string)(ct_cvar_t var);

    //! Get cvar type
    //! \param var Cvar
    //! \return Cvart type
    enum cvar_type (*get_type)(ct_cvar_t var);

    //! Set float value
    //! \param var Cvar
    //! \param f Value
    void (*set_float)(ct_cvar_t var,
                      float f);

    //! Set int value
    //! \param var Cvar
    //! \param i Value
    void (*set_int)(ct_cvar_t var,
                    int i);

    //! Set string value
    //! \param var Cvar
    //! \param s Value
    void (*set_string)(ct_cvar_t var,
                       const char *s);

    //! Dump all variables to log
    void (*log_all)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_CONFIG_H
//! \}