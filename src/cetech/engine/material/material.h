#ifndef CETECH_MATERIAL_H
#define CETECH_MATERIAL_H

#include <cetech/kernel/cdb/cdb.h>



//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Typedefs
//==============================================================================

struct ct_texture;
struct ct_cdb_obj_t;

#define MATERIAL_SHADER_PROP CT_ID64_0("shader")
#define MATERIAL_STATE_PROP CT_ID64_0("state")
#define MATERIAL_VARIABLES_PROP CT_ID64_0("variables")

#define MATERIAL_VAR_HANDLER_PROP CT_ID64_0("handler")
#define MATERIAL_VAR_NAME_PROP CT_ID64_0("name")
#define MATERIAL_VAR_TYPE_PROP CT_ID64_0("type")
#define MATERIAL_VAR_VALUE_PROP CT_ID64_0("value")

//==============================================================================
// Api
//==============================================================================

//! Material API V0
struct ct_material_a0 {

    //! Create new material
    //! \param name Material resource name
    //! \return Material
    struct ct_cdb_obj_t* (*resource_create)(uint32_t name);

    //! Set texture value
    //! \param material Material
    //! \param slot Slot Name
    //! \param texture Texture name
    void (*set_texture)(struct ct_cdb_obj_t* material,
                        uint64_t layer,
                        const char *slot,
                        uint64_t texture);

    //! Set mat44f value
    //! \param material Material
    //! \param slot Variable name
    //! \param v Value
    void (*set_mat44f)(struct ct_cdb_obj_t* material,
                       uint64_t layer,
                       const char *slot,
                       float *value);

    //! Submit material for actual render
    void (*submit)(struct ct_cdb_obj_t* material,
                   uint64_t layer,
                   uint8_t viewid);

    void (*set_texture_handler)(struct ct_cdb_obj_t* material,
                                uint64_t layer,
                                const char *slot,
                                struct ct_render_texture_handle texture);
};


#endif //CETECH_MATERIAL_H