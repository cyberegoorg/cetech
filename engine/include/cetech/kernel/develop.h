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
#include <cetech/kernel/macros.h>


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

struct ct_develop_event_header {
    uint32_t type;
    uint64_t size;
};

//! Record float event struct
struct ct_record_float_event {
    struct ct_develop_event_header header; //!< Event header
    char name[64];              //!< Entry name
    float value;                //!< Entry value
};

//! Record int event struct
struct ct_record_int_event {
    struct ct_develop_event_header header; //!< Event header
    char name[64];              //!< Entry name
    int32_t value;                  //!< Entry value
};

//! Scope event struct
struct ct_scope_event {
    struct ct_develop_event_header header; //!< Event header
    char name[64];              //!< Scope name
    time_t start;               //!< Scope create time
    float duration;             //!< Scope duration
    uint32_t depth;                  //!< Scope depth
    uint32_t worker_id;              //!< Worker id
};

//! Scope data struct
struct ct_scope_data {
    const char *name; //!< Scope name
    time_t start;     //!< Start time
    uint64_t start_timer;  //!< Timer
    uint32_t worker_id;
};

//==============================================================================
// Api
//==============================================================================

//! Develop system API V0
struct ct_develop_a0 {

    //! Push event
    //! \param header Header
    //! \param type Event type
    //! \param size Event size
    void (*push)(struct ct_develop_event_header *header,
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
    struct ct_scope_data (*enter_scope)(const char *name, uint32_t worker_id);

    //! Leave scope
    //! \param scope_data Scope data
    void (*leave_scope)(struct ct_scope_data scope_data);

    void (*after_update)(float dt);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_DEVELOP_SYSTEM_H
// \}