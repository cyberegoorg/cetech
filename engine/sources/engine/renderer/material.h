//! \addtogroup Renderer
//! \{
#ifndef CETECH_BGFX_MATERIAL_H
#define CETECH_BGFX_MATERIAL_H

#include <celib/string/stringid.h>
#include <celib/math/types.h>
#include <celib/handler/handlerid.h>

//! Material typedef
typedef struct {
    union {
        handler_t h;
        u32 idx;
    };
} material_t;


//! Material API V0
struct MaterialApiV0 {

    //! Create new material
    //! \param name Material resource name
    //! \return Material
    material_t (*resource_create)(stringid64_t name);

    //! Get texture count in material
    //! \param material Material
    //! \return Texture count
    u32 (*get_texture_count)(material_t material);

    //! Set texture value
    //! \param material Material
    //! \param slot Slot Name
    //! \param texture Texture name
    void (*set_texture)(material_t material,
                        const char *slot,
                        stringid64_t texture);

    //! Set vec4f value
    //! \param material Material
    //! \param slot Variable name
    //! \param v Value
    void (*set_vec4f)(material_t material,
                      const char *slot,
                      cel_vec4f_t v);

    //! Set mat33f value
    //! \param material Material
    //! \param slot Variable name
    //! \param v Value
    void (*set_mat33f)(material_t material,
                       const char *slot,
                       mat33f_t v);

    //! Set mat44f value
    //! \param material Material
    //! \param slot Variable name
    //! \param v Value
    void (*set_mat44f)(material_t material,
                       const char *slot,
                       cel_mat44f_t v);

    //! Use material for actual render
    void (*use)(material_t material);

    //! Submit material for actual render
    void (*submit)(material_t material);
};


#endif //CETECH_BGFX_MATERIAL_H
//! \}