//! \defgroup Application
//! Main application
//! \{
#ifndef CETECH_APPLICATION_API_H
#define CETECH_APPLICATION_API_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Include
//==============================================================================

struct ct_window;

//==============================================================================
// Struct
//==============================================================================

typedef void (*ct_app_on_init)();
typedef void (*ct_app_on_shutdown)();
typedef void (*ct_app_on_update)(float dt);
typedef void (*ct_app_on_render)();

//==============================================================================
// Api
//==============================================================================

//! Application API V0
struct ct_app_a0 {
    //! Stop main loop and quit.
    void (*quit)();

    void (*start)();

    void (*register_on_init)(ct_app_on_init on_init);

    void (*unregister_on_init)(ct_app_on_init on_init);

    void (*register_on_shutdown)(ct_app_on_shutdown on_shutdown);

    void (*unregister_on_shutdown)(ct_app_on_shutdown on_shutdown);

    void (*register_on_update)(ct_app_on_update on_update);

    void (*unregister_on_update)(ct_app_on_update on_update);

    void (*register_on_render)(ct_app_on_render on_render);

    void (*unregister_on_render)(ct_app_on_render on_render);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_APPLICATION_H
// \}