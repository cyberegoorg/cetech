#ifndef CETECH_APPLICATION_H
#define CETECH_APPLICATION_H

//==============================================================================
// Interface
//==============================================================================

//! Init application
//! \param argc Arg count
//! \param argv Args array
//! \return 1 if ok else error code
int application_init(int argc, char **argv);

//! Shutdown application
void application_shutdown();

//! Start application main loop
void application_start();

//! Stop main loop and quit.
void application_quit();

#endif //CETECH_APPLICATION_H
