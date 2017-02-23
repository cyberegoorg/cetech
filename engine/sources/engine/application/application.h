//! \defgroup Application
//! Main application
//! \{
#ifndef CETECH_APPLICATION_H
#define CETECH_APPLICATION_H

#include "celib/window/types.h"

struct game_callbacks {
    int(*init)();
    void(*shutdown)();
    void(*update)(float dt);
    void(*render)();
};

//==============================================================================
// Interface
//==============================================================================

//! Init application
//! \param argc Arg count
//! \param argv Args array
//! \return 1 if ok else error code
int application_init(int argc,
                     const char **argv);

//! Shutdown application
int application_shutdown();

//! Start application main loop
void application_start();

//! Application API V0
struct ApplicationApiV0 {
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
    cel_window_t (*main_window)();
};

#endif //CETECH_APPLICATION_H
// \}