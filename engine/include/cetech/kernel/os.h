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

#include <cetech/celib/math_types.h>

#include "_machine_enums.h"

struct ct_allocator;

//==============================================================================
// Typedefs
//==============================================================================

typedef int (*ct_thread_fce_t)(void *data);

//==============================================================================
// Enums
//==============================================================================

enum ct_vio_open_mode {
    VIO_OPEN_READ,
    VIO_OPEN_WRITE,
};

enum ct_vio_seek {
    VIO_SEEK_SET,
    VIO_SEEK_CUR,
    VIO_SEEK_END
};

enum ct_window_flags {
    WINDOW_NOFLAG = 0,
    WINDOW_FULLSCREEN = 1,
};

enum ct_window_pos {
    WINDOWPOS_CENTERED = 1,
    WINDOWPOS_UNDEFINED = 2
};

//==============================================================================
// Structs
//==============================================================================

typedef void ct_thread_t;

typedef struct {
    int lock;
} ct_spinlock_t;

typedef void os_window_t;

typedef void ct_vio_instance;

struct ct_vio  {
    ct_vio_instance* inst;

    int64_t (*size)(ct_vio_instance *vio);

    int64_t (*seek)(ct_vio_instance *vio,
                    int64_t offset,
                    enum ct_vio_seek whence);

    size_t (*read)(ct_vio_instance *vio,
                   void *ptr,
                   size_t size,
                   size_t maxnum);

    size_t (*write)(ct_vio_instance *vio,
                    const void *ptr,
                    size_t size,
                    size_t num);

    int (*close)(ct_vio_instance *vio);
};

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
    vec2f_t pos;       //!< Actual position
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
    vec2f_t position;  //!< Position
};

//! Gamepad button event
struct gamepad_btn_event {
    struct ct_event_header h; //!< Event header
    uint8_t gamepad_id;         //!< Gamepad id
    uint32_t button;            //!< Button state
};

//! Gamepad device event
struct ct_gamepad_device_event {
    struct ct_event_header h; //!< Event header
    uint8_t gamepad_id;         //!< Gamepad id
};

//==============================================================================
// Api
//==============================================================================

struct ct_cpu_a0 {
    int (*count)();
};

struct ct_object_a0 {
    void *(*load)(const char *path);

    void (*unload)(void *so);

    void *(*load_function)(void *so,
                           const char *name);
};

struct ct_path_a0 {
    //! Get file modified time
    //! \param path File path
    //! \return Modified time
    uint32_t (*file_mtime)(const char *path);

    //! List dir
    //! \param path Dir path
    //! \param recursive Resucrsive list?
    //! \param files Result files
    //! \param allocator Allocator
    void (*list)(const char *path,
                 int recursive,
                 char ***files,
                 uint32_t *count,
                 struct ct_allocator *allocator);

    //! Free list dir array
    //! \param files Files array
    //! \param allocator Allocator
    void (*list_free)(char **files,
                      uint32_t count,
                      struct ct_allocator *allocator);

    //! Create dir path
    //! \param path Path
    //! \return 1 of ok else 0
    int (*make_path)(const char *path);

    //! Get filename from path
    //! \param path Path
    //! \return Filename
    const char *(*filename)(const char *path);

    //! Get file basename (filename without extension)
    //! \param path Path
    //! \param out Out basename
    //! \param size
    void (*basename)(const char *path,
                     char *out,
                     size_t size);

    void (*dir)(char *out,
                size_t size,
                const char *path);

    //! Get file extension
    //! \param path Path
    //! \return file extension
    const char *(*extension)(const char *path);

    //! Join paths
    //! \param allocator Allocator
    //! \param count Path count.
    //! \return Result path len.
    char *(*join)(struct ct_allocator *allocator,
                  uint32_t count,
                  ...);
};

struct ct_process_a0 {
    int (*exec)(const char *argv);
};

struct ct_thread_a0 {
    //! Create new thread
    //! \param fce Thread fce
    //! \param name Thread name
    //! \param data Thread data
    //! \return new thread
    ct_thread_t *(*create)(ct_thread_fce_t fce,
                           const char *name,
                           void *data);

    //! Kill thread
    //! \param thread thread
    void (*kill)(ct_thread_t *thread);

    //! Wait for thread
    //! \param thread Thread
    //! \param status Thread exit status
    void (*wait)(ct_thread_t *thread,
                 int *status);

    //! Get id for thread
    //! \param thread Thread
    //! \return ID
    uint64_t (*get_id)(ct_thread_t *thread);

    //! Get actual thread id
    //! \return Thread id
    uint64_t (*actual_id)();

    void (*yield)();

    void (*spin_lock)(ct_spinlock_t *lock);

    void (*spin_unlock)(ct_spinlock_t *lock);
};

struct ct_time_a0 {
    uint32_t (*ticks)();

    uint64_t (*perf_counter)();

    uint64_t (*perf_freq)();
};

struct ct_vio_a0 {
    struct ct_vio *(*from_file)(const char *path,
                                enum ct_vio_open_mode mode);
};

struct ct_window_a0 {
    os_window_t *(*create)(const char *title,
                           enum ct_window_pos x,
                           enum ct_window_pos y,
                           const int32_t width,
                           const int32_t height,
                           enum ct_window_flags flags);

    os_window_t *(*create_from)(void *hndl);

    void (*destroy)(os_window_t *w);

    void (*set_title)(os_window_t *w,
                      const char *title);

    const char *(*get_title)(os_window_t *w);

    void (*update)(os_window_t *w);

    void (*resize)(os_window_t *w,
                   uint32_t width,
                   uint32_t height);

    void (*size)(os_window_t *window,
                 uint32_t *width,
                 uint32_t *height);

    void *(*native_window_ptr)(os_window_t *w);

    void *(*native_display_ptr)(os_window_t *w);
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
