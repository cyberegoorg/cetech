//! \ingroup Develop
//! \{
#ifndef CETECH_CONSOLE_SERVER_API_H
#define CETECH_CONSOLE_SERVER_API_H

#include "include/mpack/mpack.h"

//==============================================================================
// Interface
//==============================================================================

//! Console server command function
typedef int (*console_server_command_t)(mpack_node_t,
                                        mpack_writer_t *);

//! Console server API V1
struct ConsoleServerApiV1 {
    //! Push begin signal to clients
    void (*consolesrv_push_begin)();

    //! Register consoleserver command
    //! \param name Command name ex.: "lua.execute"
    //! \param command Command fce
    void (*consolesrv_register_command)(const char * name,
                                        console_server_command_t command);
};

#endif //CETECH_CONSOLE_SERVER_API_H
// \}