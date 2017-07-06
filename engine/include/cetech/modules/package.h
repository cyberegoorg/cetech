//! \defgroup Resource
//! Resource system
//! \{
#ifndef CETECH_PACKAGE_H
#define CETECH_PACKAGE_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Api
//==============================================================================

//! Package API V!
struct ct_package_a0 {
    //! Load package
    //! \param name Package name
    void (*load)(uint64_t name);

    //! Unload package
    //! \param name Package name
    void (*unload)(uint64_t name);

    //! Is package loaded
    //! \param name Package name
    int (*is_loaded)(uint64_t name);

    //! Wait while not package loaded.
    //! \param name Package name
    void (*flush)(uint64_t name);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_PACKAGE_H
//! |}