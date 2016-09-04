#ifndef CETECH_APPLICATION_H
#define CETECH_APPLICATION_H

typedef int(*game_init_clb_t)();

typedef void(*game_shutdown_clb_t)();

typedef void(*game_update_clb_t)(float dt);

typedef void(*game_render_clb_t)();

struct game_callbacks {
    game_init_clb_t init;
    game_shutdown_clb_t shutdown;
    game_update_clb_t update;
    game_render_clb_t render;
};

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

const char *application_platform();

const char *application_native_platform();

#endif //CETECH_APPLICATION_H
