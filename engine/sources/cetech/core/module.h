//! \addtogroup Plugin
//! \{
#ifndef CETECH_PLUGIN_API_H
#define CETECH_PLUGIN_API_H

//==============================================================================
// Includes
//==============================================================================

//==============================================================================
// Defines
//==============================================================================

#define IMPORT_API(name) static struct name name;
#define INIT_API(get_api, name, api_id) name = *(struct name*) get_api(api_id)


//==============================================================================
// Enums
//==============================================================================

enum {
    PLUGIN_EXPORT_API_ID = 1,
};

struct config_api_v0;

//==============================================================================
// Typedefs
//==============================================================================

typedef void *(*get_api_fce_t)(int api);


//==============================================================================
// Api
//==============================================================================

//! Plugin expot api struct V0
struct module_api_v0 {

    //! Init cvars
    //! \param Init cvars
    void (*init_cvar       )(struct config_api_v0);

    //! Init
    //! \param get_api_fce_t Get engine api
    void (*init            )(get_api_fce_t);

    //! Shutdown
    void (*shutdown        )(void);

    //! Reload begin
    //! \param get_api_fce_t Get engine api
    void *(*reload_begin   )(get_api_fce_t);

    //! Reload end
    //! \param get_api_fce_t Get engine api
    void (*reload_end      )(get_api_fce_t,
                             void *);

    //! Call update
    void (*update          )();

    //! After update
    //! \param dt Deltatime
    void (*after_update    )(float dt);
};

#endif //CETECH_PLUGIN_API_H
//! \}