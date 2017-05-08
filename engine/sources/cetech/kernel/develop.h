//! \defgroup Develop
//! Console server, develop system

#ifndef CETECH_CONSOLE_SERVER_API_H
#define CETECH_CONSOLE_SERVER_API_H

//==============================================================================
// Includes
//==============================================================================

#include "include/mpack/mpack.h"

#include "../core/types.h"
#include "../core/eventstream.inl"

enum {
    CONSOLE_SERVER_API_ID = 9,
    DEVELOP_SERVER_API_ID = 10
};

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
struct cnsole_srv_api_v0 {
    //! Push begin signal to clients
    void (*consolesrv_push_begin)();

    //! Register consoleserver command
    //! \param name Command name ex.: "lua.execute"
    //! \param command Command fce
    void (*consolesrv_register_command)(const char *name,
                                        console_server_command_t command);
};


//==============================================================================
// Enums
//==============================================================================

//! Event type enum
enum {
    EVENT_NULL = 0,     //! Invalid event
    EVENT_SCOPE,        //! Scope event
    EVENT_RECORD_FLOAT, //! Record float event
    EVENT_RECORD_INT,   //! Record int event
};

//==============================================================================
// Structs
//==============================================================================

//! Record float event struct
struct record_float_event {
    struct event_header header; //!< Event header
    char name[64];              //!< Entry name
    float value;                //!< Entry value
};

//! Record int event struct
struct record_int_event {
    struct event_header header; //!< Event header
    char name[64];              //!< Entry name
    int32_t value;                  //!< Entry value
};

//! Scope event struct
struct scope_event {
    struct event_header header; //!< Event header
    char name[64];              //!< Scope name
    time_t start;               //!< Scope create time
    float duration;             //!< Scope duration
    uint32_t depth;                  //!< Scope depth
    uint32_t worker_id;              //!< Worker id
};

//! Scope data struct
struct scope_data {
    const char *name; //!< Scope name
    time_t start;     //!< Start time
    uint64_t start_timer;  //!< Timer
};

//==============================================================================
// Api
//==============================================================================

//! Develop system API V0
struct develop_api_v0 {

    //! Push event
    //! \param header Header
    //! \param type Event type
    //! \param size Event size
    void (*push)(struct event_header *header,
                 uint32_t type,
                 uint64_t size);

    //! Push float value
    //! \param name Value name
    //! \param value Values
    void (*push_record_float)(const char *name,
                              float value);

    //! Push int value
    //! \param name Value name
    //! \param value Values
    void (*push_record_int)(const char *name,
                            int value);

    //! Enter scope
    //! \param name Scope name
    //! \return Scope data
    struct scope_data (*enter_scope)(const char *name);

    //! Leave scope
    //! \param scope_data Scope data
    void (*leave_scope)(struct scope_data scope_data);
};

#endif //CETECH_CONSOLE_SERVER_API_H
// \}