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


//==============================================================================
// Api
//==============================================================================

//! Application API V0
struct ct_app_a0 {
    //! Stop main loop and quit.
    void (*quit)();

    //! Get platform
    //! \return Platform platfrom
    const char *(*platform)();

    //! Get native platform
    //! \return Native platfrom
    const char *(*native_platform)();

    //! Get main window
    //! \return Main window
    ct_window *(*main_window)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_APPLICATION_H
// \}