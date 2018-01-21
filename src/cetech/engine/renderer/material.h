#ifndef CETECH_MATERIAL_H
#define CETECH_MATERIAL_H

#include <cetech/core/coredb/coredb.h>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Typedefs
//==============================================================================

struct ct_texture;
struct ct_cdb_object_t;

struct ct_material {
    uint64_t idx;
};

//==============================================================================
// Api
//==============================================================================

//! Material API V0
struct ct_material_a0 {

    //! Create new material
    //! \param name Material resource name
    //! \return Material
    struct ct_cdb_object_t* (*resource_create)(uint64_t name);

    //! Set texture value
    //! \param material Material
    //! \param slot Slot Name
    //! \param texture Texture name
    void (*set_texture)(struct ct_cdb_object_t* material,
                        uint64_t layer,
                        const char *slot,
                        uint64_t texture);

    //! Set mat44f value
    //! \param material Material
    //! \param slot Variable name
    //! \param v Value
    void (*set_mat44f)(struct ct_cdb_object_t* material,
                       uint64_t layer,
                       const char *slot,
                       float *value);

    //! Submit material for actual render
    void (*submit)(struct ct_cdb_object_t* material,
                   uint64_t layer,
                   uint8_t viewid);

    void (*set_texture_handler)(struct ct_cdb_object_t* material,
                                uint64_t layer,
                                const char *slot,
                                struct ct_texture texture);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_MATERIAL_H