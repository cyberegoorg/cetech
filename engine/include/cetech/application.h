//! \defgroup Application
//! Main application
//! \{
#ifndef CETECH_APPLICATION_API_H
#define CETECH_APPLICATION_API_H

//==============================================================================
// Include
//==============================================================================

#include "window.h"


enum {
    APPLICATION_API_ID = 2,
};

//==============================================================================
// Struct
//==============================================================================

struct game_callbacks {
    int (*init)();

    void (*shutdown)();

    void (*update)(float dt);

    void (*render)();
};


//==============================================================================
// Api
//==============================================================================

//! Application API V0
struct app_api_v0 {
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
    window_t (*main_window)();
};

#endif //CETECH_APPLICATION_H
// \}