//! \defgroup Develop
//! Console server, develop system

#ifndef CETECH_CONSOLE_SYSTEM_H
#define CETECH_CONSOLE_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <cetech/kernel/macros.h>

typedef struct mpack_node_t mpack_node_t;
typedef struct mpack_writer_t mpack_writer_t;

//==============================================================================
// Typedefs
//==============================================================================

//! Console server command function
typedef int (*ct_console_srv_command_t)(mpack_node_t,
                                        mpack_writer_t *);

//==============================================================================
// Api
//==============================================================================

//! Console server API V0
struct ct_console_srv_api_v0 {
    //! Push begin signal to clients
    void (*push_begin)();

    //! Register consoleserver command
    //! \param name Command name ex.: "lua.execute"
    //! \param command Command fce
    void (*register_command)(const char *name,
                             ct_console_srv_command_t command);

    void (*update)();
};


#ifdef __cplusplus
}
#endif

#endif //CETECH_CONSOLE_SYSTEM_H
// \}