//                          **OS layer abstraction**
//

#ifndef CETECH_OS_H
#define CETECH_OS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "module.inl"

struct ct_alloc;

#ifdef CETECH_DEBUG
#define CETECH_ASSERT(where, condition)                                 \
    do {                                                                \
        if (!(condition)) {                                             \
            ct_os_a0->error->assert(where, #condition, __FILE__, __LINE__); \
        }                                                               \
    } while (0)
#else
#define CETECH_ASSERT(where, condition) \
    do {} while (0)
#endif

// # Error

struct ct_os_error_a0 {
    // Assert
    void (*assert)(const char *where,
                   const char *condition,
                   const char *filename,
                   int line);
};

// # CPU

struct ct_os_cpu_a0 {
    // Get cpu core count
    int (*count)();
};


// # Object

struct ct_os_object_a0 {
    // Load shared lib
    void *(*load)(const char *path);

    // Unload shared lib
    void (*unload)(void *so);

    // Load function from shared lib
    void *(*load_function)(void *so,
                           const char *name);
};


// # Path

struct ct_os_path_a0 {
    // Get file modified time
    uint32_t (*file_mtime)(const char *path);

    // List dir
    // - path Dir path
    // - recursive Resucrsive list?
    // - files Result files
    // - allocator Allocator
    void (*list)(const char *path,
                 const char **patern,
                 uint32_t patern_n,
                 int recursive,
                 int only_dir,
                 char ***files,
                 uint32_t *count,
                 struct ct_alloc *allocator);

    // Free list dir array
    // - files Files array
    // - allocator Allocator
    void (*list_free)(char **files,
                      uint32_t count,
                      struct ct_alloc *allocator);

    // Create dir path
    // - path Path
    int (*make_path)(const char *path);

    // Get filename from path
    // - path Path
    const char *(*filename)(const char *path);

    // Get file basename (filename without extension)
    // - path Path
    // - out Out basename
    // - size
    void (*basename)(const char *path,
                     char *out);

    void (*dir)(char *out,
                const char *path);

    void (*dirname)(char *out,
                    const char *path);

    // Get file extension
    // - path Path
    const char *(*extension)(const char *path);

    // Join paths and return path len.
    // - allocator Allocator
    // - count Path count.
    void (*join)(char **buffer,
                 struct ct_alloc *allocator,
                 uint32_t count,
                 ...);

    void (*copy_file)(struct ct_alloc *allocator,
                      const char *from,
                      const char *to);

    bool (*is_dir)(const char *path);
};



// # Process

struct ct_os_process_a0 {
    int (*exec)(const char *argv);
};


// # Thread

typedef void ct_thread_t;

typedef int (*ct_thread_fce_t)(void *data);

struct ct_spinlock {
    int lock;
};

struct ct_os_thread_a0 {
    // Create new thread
    // - fce Thread fce
    // - name Thread name
    // - data Thread data
    ct_thread_t *(*create)(ct_thread_fce_t fce,
                           const char *name,
                           void *data);

    // Kill thread
    // - thread thread
    void (*kill)(ct_thread_t *thread);

    // Wait for thread
    // - thread Thread
    // - status Thread exit status
    void (*wait)(ct_thread_t *thread,
                 int *status);

    // Get id for thread
    // - thread Thread
    uint64_t (*get_id)(ct_thread_t *thread);

    // Get actual thread id
    uint64_t (*actual_id)();

    void (*yield)();

    void (*spin_lock)(struct ct_spinlock *lock);

    void (*spin_unlock)(struct ct_spinlock *lock);
};



// # Time

struct ct_os_time_a0 {
    uint32_t (*ticks)();

    uint64_t (*perf_counter)();

    uint64_t (*perf_freq)();
};



// # VIO

enum ct_vio_open_mode {
    VIO_OPEN_READ,
    VIO_OPEN_WRITE,
};

enum ct_vio_seek {
    VIO_SEEK_SET,
    VIO_SEEK_CUR,
    VIO_SEEK_END
};

typedef void ct_vio_instance_t;

struct ct_vio {
    ct_vio_instance_t *inst;

    int64_t (*size)(struct ct_vio *vio);

    int64_t (*seek)(struct ct_vio *vio,
                    int64_t offset,
                    enum ct_vio_seek whence);

    size_t (*read)(struct ct_vio *vio,
                   void *ptr,
                   size_t size,
                   size_t maxnum);

    size_t (*write)(struct ct_vio *vio,
                    const void *ptr,
                    size_t size,
                    size_t num);

    int (*close)(struct ct_vio *vio);
};

struct ct_os_vio_a0 {
    struct ct_vio *(*from_file)(const char *path,
                                enum ct_vio_open_mode mode);
};


// # Watchdog

enum ct_watchdog_ev_type {
    CT_WATCHDOG_EVENT_NONE = 0,
    CT_WATCHDOG_EVENT_FILE_MODIFIED = 1
};

struct ct_watchdog_ev_header {
    uint32_t type;
    uint64_t size;
};

struct ct_wd_ev_file_write_end {
    struct ct_watchdog_ev_header header;
    const char *dir;
    char *filename;
};


typedef void ct_watchdog_instance_t;

struct ct_watchdog {
    ct_watchdog_instance_t *inst;

    void (*add_dir)(ct_watchdog_instance_t *inst,
                    const char *path,
                    bool recursive);

    void (*fetch_events)(ct_watchdog_instance_t *inst);

    struct ct_watchdog_ev_header *(*event_begin)(ct_watchdog_instance_t *inst);

    struct ct_watchdog_ev_header *(*event_end)(ct_watchdog_instance_t *inst);

    struct ct_watchdog_ev_header *(*event_next)(ct_watchdog_instance_t *inst,
                                                struct ct_watchdog_ev_header *header);
};

struct ct_os_watchdog_a0 {
    struct ct_watchdog *(*create)(struct ct_alloc *alloc);

    void (*destroy)(struct ct_watchdog *watchdog);
};


// # Window

#define WINDOW_EBUS_NAME "window"

enum {
    WINDOW_EBUS = 0x7a0d633e
};

enum {
    EVENT_WINDOW_INVALID = 0,   //< Invalid type

    EVENT_WINDOW_RESIZED, //< Window resized
};

struct ct_window_resized_event {
    uint32_t window_id;
    int32_t width;
    int32_t height;
};

enum ct_window_flags {
    WINDOW_NOFLAG = (1 << 0),
    WINDOW_FULLSCREEN = (1 << 1),
    WINDOW_SHOWN = (1 << 2),
    WINDOW_HIDDEN = (1 << 3),
    WINDOW_BORDERLESS = (1 << 4),
    WINDOW_RESIZABLE = (1 << 5),
    WINDOW_MINIMIZED = (1 << 6),
    WINDOW_MAXIMIZED = (1 << 7),
    WINDOW_INPUT_GRABBED = (1 << 8),
    WINDOW_INPUT_FOCUS = (1 << 9),
    WINDOW_MOUSE_FOCUS = (1 << 10),
    WINDOW_FULLSCREEN_DESKTOP = (1 << 11),
    WINDOW_ALLOW_HIGHDPI = (1 << 12),
    WINDOW_MOUSE_CAPTURE = (1 << 13),
    WINDOW_ALWAYS_ON_TOP = (1 << 14),
    WINDOW_SKIP_TASKBAR = (1 << 15),
    WINDOW_UTILITY = (1 << 16),
    WINDOW_TOOLTIP = (1 << 17),
    WINDOW_POPUP_MENU = (1 << 18),
};

enum ct_window_pos {
    WINDOWPOS_NOFLAG = (1 << 0),
    WINDOWPOS_CENTERED = (1 << 1),
    WINDOWPOS_UNDEFINED = (1 << 2),
};

typedef void ct_window_ints;

struct ct_window {
    ct_window_ints *inst;

    void (*set_title)(ct_window_ints *w,
                      const char *title);

    const char *(*get_title)(ct_window_ints *w);

    void (*resize)(ct_window_ints *w,
                   uint32_t width,
                   uint32_t height);

    void (*size)(ct_window_ints *window,
                 uint32_t *width,
                 uint32_t *height);

    void *(*native_window_ptr)(ct_window_ints *w);

    void *(*native_display_ptr)(ct_window_ints *w);
};


struct ct_os_window_a0 {
    struct ct_window *(*create)(struct ct_alloc *alloc,
                                const char *title,
                                enum ct_window_pos x,
                                enum ct_window_pos y,
                                const int32_t width,
                                const int32_t height,
                                uint32_t flags);

    struct ct_window *(*create_from)(struct ct_alloc *alloc,
                                     void *hndl);

    void (*destroy)(struct ct_alloc *alloc,
                    struct ct_window *w);
};

struct ct_os_a0 {
    struct ct_os_cpu_a0 *cpu;
    struct ct_os_error_a0 *error;
    struct ct_os_object_a0 *object;
    struct ct_os_path_a0 *path;
    struct ct_os_process_a0 *process;
    struct ct_os_thread_a0 *thread;
    struct ct_os_time_a0 *time;
    struct ct_os_vio_a0 *vio;
    struct ct_os_watchdog_a0 *watchdog;
    struct ct_os_window_a0 *window;
};

CT_MODULE(ct_os_a0);


#endif //CETECH_OS_H
