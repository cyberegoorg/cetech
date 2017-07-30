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

struct ct_game_callbacks {
    int (*init)();

    void (*shutdown)();

    void (*update)(float dt);

    void (*render)();
};

typedef void (*ct_app_on_update)(float dt);

//==============================================================================
// Api
//==============================================================================

//! Application API V0
struct ct_app_a0 {
    //! Stop main loop and quit.
    void (*quit)();
    void (*start)();

    void (*register_on_update)(ct_app_on_update on_update);
    void (*unregister_on_update)(ct_app_on_update on_update);

};

#ifdef __cplusplus
}
#endif

#endif //CETECH_APPLICATION_H
// \}