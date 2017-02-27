//! \ingroup Develop
//! \{
#ifndef CETECH_CONSOLE_SERVER_API_H
#define CETECH_CONSOLE_SERVER_API_H

//==============================================================================
// Includes
//==============================================================================

#include "include/mpack/mpack.h"

//==============================================================================
// Typedefs
//==============================================================================

//! Console server command function
typedef int (*console_server_command_t)(mpack_node_t,
                                        mpack_writer_t *);

//==============================================================================
// Api
//==============================================================================

//! Console server API V0
struct ConsoleServerApiV0 {
    //! Push begin signal to clients
    void (*consolesrv_push_begin)();

    //! Register consoleserver command
    //! \param name Command name ex.: "lua.execute"
    //! \param command Command fce
    void (*consolesrv_register_command)(const char *name,
                                        console_server_command_t command);
};

#endif //CETECH_CONSOLE_SERVER_API_H
// \}