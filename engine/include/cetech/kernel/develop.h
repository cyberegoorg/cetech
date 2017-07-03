//! \defgroup Develop
//! Console server, develop system

#ifndef CETECH_DEVELOP_SYSTEM_H
#define CETECH_DEVELOP_SYSTEM_H


#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <time.h>
#include <cetech/kernel/types.h>


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

struct develop_event_header {
    uint32_t type;
    uint64_t size;
};

//! Record float event struct
struct record_float_event {
    struct develop_event_header header; //!< Event header
    char name[64];              //!< Entry name
    float value;                //!< Entry value
};

//! Record int event struct
struct record_int_event {
    struct develop_event_header header; //!< Event header
    char name[64];              //!< Entry name
    int32_t value;                  //!< Entry value
};

//! Scope event struct
struct scope_event {
    struct develop_event_header header; //!< Event header
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
    void (*push)(struct develop_event_header *header,
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

    void (*after_update)(float dt);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_DEVELOP_SYSTEM_H
// \}