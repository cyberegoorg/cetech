//! \defgroup Application
//! Main application
//! \{
#ifndef CETECH_APPLICATION_H
#define CETECH_APPLICATION_H

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

#endif //CETECH_APPLICATION_H
// \}