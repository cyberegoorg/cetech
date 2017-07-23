#ifndef CETECH_OS_H
#define CETECH_OS_H


#ifdef __cplusplus
extern "C" {
#endif


//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <stddef.h>

#include "_machine_enums.h"

struct cel_alloc;

//==============================================================================
// Window
//==============================================================================

enum ct_window_flags {
    WINDOW_NOFLAG = 0,
    WINDOW_FULLSCREEN = 1,
};

enum ct_window_pos {
    WINDOWPOS_CENTERED = 1,
    WINDOWPOS_UNDEFINED = 2
};

typedef void ct_window_t;

struct ct_window_a0 {
    ct_window_t *(*create)(const char *title,
                           enum ct_window_pos x,
                           enum ct_window_pos y,
                           const int32_t width,
                           const int32_t height,
                           enum ct_window_flags flags);

    ct_window_t *(*create_from)(void *hndl);

    void (*destroy)(ct_window_t *w);

    void (*set_title)(ct_window_t *w,
                      const char *title);

    const char *(*get_title)(ct_window_t *w);

    void (*update)(ct_window_t *w);

    void (*resize)(ct_window_t *w,
                   uint32_t width,
                   uint32_t height);

    void (*size)(ct_window_t *window,
                 uint32_t *width,
                 uint32_t *height);

    void *(*native_window_ptr)(ct_window_t *w);

    void *(*native_display_ptr)(ct_window_t *w);
};

//==============================================================================
// Machine
//==============================================================================

struct ct_event_header {
    uint32_t type;
    uint64_t size;
};

//! Mouse button status
struct ct_mouse_event {
    struct ct_event_header h; //!< Event header
    uint32_t button;            //!< Button state
};

struct ct_mouse_move_event {
    struct ct_event_header h; //!< Event header
    float pos[2];       //!< Actual position
};

//! Keyboard event
struct ct_keyboard_event {
    struct ct_event_header h; //!< Event header
    uint32_t keycode; //!< Key code
};

//! Gamepad move event
struct ct_gamepad_move_event {
    struct ct_event_header h; //!< Event header
    uint8_t gamepad_id;         //!< Gamepad id
    uint32_t axis;              //!< Axis id
    float position[2];  //!< Position
};

//! Gamepad button event
struct ct_gamepad_btn_event {
    struct ct_event_header h; //!< Event header
    uint8_t gamepad_id;         //!< Gamepad id
    uint32_t button;            //!< Button state
};

//! Gamepad device event
struct ct_gamepad_device_event {
    struct ct_event_header h; //!< Event header
    uint8_t gamepad_id;         //!< Gamepad id
};

//! Machine API V0
struct ct_machine_a0 {

    //! Get eventstream begin
    //! \return Begin
    struct ct_event_header *(*event_begin)();

    //! Get eventstream end
    //! \return End
    struct ct_event_header *(*event_end)();

    //! Next event
    //! \param header Actual event header
    //! \return Next event header
    struct ct_event_header *(*event_next)(struct ct_event_header *header);

    //! Is gamepad active?
    //! \param gamepad Gamepad ID
    //! \return 1 if is active else 0
    int (*gamepad_is_active)(int gamepad);

    //!
    //! \param gamepad
    //! \param strength
    //! \param length
    void (*gamepad_play_rumble)(int gamepad,
                                float strength,
                                uint32_t length);

    void (*update)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_OS_H
