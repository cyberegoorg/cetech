//                          **OS layer abstraction**
//

#ifndef CE_OS_H
#define CE_OS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "module.inl"

#define CE_OS_API \
    CE_ID64_0("ce_os_a0", 0x1f1e6d0d81d750baULL)

#ifdef DEBUG

#define CE_ASSERT(where, condition)                                 \
    do {                                                                \
        if (!(condition)) {                                             \
            ce_os_a0->error->assert(where, #condition, __FILE__, __LINE__); \
        }                                                               \
    } while (0)
#else
#define CE_ASSERT(where, condition) \
    do {} while (0)
#endif

struct ce_alloc;

// # Error

struct ce_os_error_a0 {
    // Assert
    void (*assert)(const char *where,
                   const char *condition,
                   const char *filename,
                   int line);

    char *(*stacktrace)(int skip);

    void (*stacktrace_free)(char *st);
};

// # CPU

struct ce_os_cpu_a0 {
    // Get cpu core count
    int (*count)();
};


// # Object

struct ce_os_object_a0 {
    // Load shared lib
    void *(*load)(const char *path);

    // Unload shared lib
    void (*unload)(void *so);

    // Load function from shared lib
    void *(*load_function)(void *so,
                           const char *name);
};


// # Path

struct ce_os_path_a0 {
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
                 struct ce_alloc *allocator);

    // Free list dir array
    // - files Files array
    // - allocator Allocator
    void (*list_free)(char **files,
                      uint32_t count,
                      struct ce_alloc *allocator);

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
                 struct ce_alloc *allocator,
                 uint32_t count,
                 ...);

    void (*copy_file)(struct ce_alloc *allocator,
                      const char *from,
                      const char *to);

    bool (*is_dir)(const char *path);
};



// # Process

struct ce_os_process_a0 {
    int (*exec)(const char *argv);
};


// # Thread

typedef void ce_thread_t;

typedef int (*ce_thread_fce_t)(void *data);

struct ce_spinlock {
    int lock;
};

struct ce_os_thread_a0 {
    // Create new thread
    // - fce Thread fce
    // - name Thread name
    // - data Thread data
    ce_thread_t *(*create)(ce_thread_fce_t fce,
                           const char *name,
                           void *data);

    // Kill thread
    // - thread thread
    void (*kill)(ce_thread_t *thread);

    // Wait for thread
    // - thread Thread
    // - status Thread exit status
    void (*wait)(ce_thread_t *thread,
                 int *status);

    // Get id for thread
    // - thread Thread
    uint64_t (*get_id)(ce_thread_t *thread);

    // Get actual thread id
    uint64_t (*actual_id)();

    void (*yield)();

    void (*spin_lock)(struct ce_spinlock *lock);

    void (*spin_unlock)(struct ce_spinlock *lock);
};



// # Time

struct ce_os_time_a0 {
    uint32_t (*ticks)();

    uint64_t (*perf_counter)();

    uint64_t (*perf_freq)();
};



// # VIO

enum ce_vio_open_mode {
    VIO_OPEN_READ,
    VIO_OPEN_WRITE,
};

enum ce_vio_seek {
    VIO_SEEK_SET = 1,
    VIO_SEEK_CUR,
    VIO_SEEK_END
};

typedef void ce_vio_instance_t;

struct ce_vio {
    ce_vio_instance_t *inst;

    int64_t (*size)(struct ce_vio *vio);

    int64_t (*seek)(struct ce_vio *vio,
                    int64_t offset,
                    enum ce_vio_seek whence);

    size_t (*read)(struct ce_vio *vio,
                   void *ptr,
                   size_t size,
                   size_t maxnum);

    size_t (*write)(struct ce_vio *vio,
                    const void *ptr,
                    size_t size,
                    size_t num);

    int (*close)(struct ce_vio *vio);
};

struct ce_os_vio_a0 {
    struct ce_vio *(*from_file)(const char *path,
                                enum ce_vio_open_mode mode);
};



// # Window

enum {
    EVENT_WINDOW_INVALID = 0,   //< Invalid type

    EVENT_WINDOW_RESIZED = 13, //< Window resized
};

struct ce_window_resized_event {
    uint32_t window_id;
    int32_t width;
    int32_t height;
};

enum ce_window_flags {
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

enum ce_window_pos {
    WINDOWPOS_NOFLAG = (1 << 0),
    WINDOWPOS_CENTERED = (1 << 1),
    WINDOWPOS_UNDEFINED = (1 << 2),
};

typedef void ce_window_ints;

struct ce_window {
    ce_window_ints *inst;

    void (*set_title)(ce_window_ints *w,
                      const char *title);

    const char *(*get_title)(ce_window_ints *w);

    void (*resize)(ce_window_ints *w,
                   uint32_t width,
                   uint32_t height);

    void (*size)(ce_window_ints *window,
                 uint32_t *width,
                 uint32_t *height);

    void *(*native_window_ptr)(ce_window_ints *w);

    void *(*native_display_ptr)(ce_window_ints *w);

    void (*warp_mouse)(ce_window_ints *window,
                       int x,
                       int y);
};


struct ce_os_window_a0 {
    struct ce_window *(*create)(struct ce_alloc *alloc,
                                const char *title,
                                enum ce_window_pos x,
                                enum ce_window_pos y,
                                const int32_t width,
                                const int32_t height,
                                uint32_t flags);

    struct ce_window *(*create_from)(struct ce_alloc *alloc,
                                     void *hndl);

    void (*destroy)(struct ce_alloc *alloc,
                    struct ce_window *w);
};

struct ce_os_input_a0 {
    const char *(*get_clipboard_text)();
    void (*set_clipboard_text)(const char *text);
};

struct ce_os_a0 {
    struct ce_os_cpu_a0 *cpu;
    struct ce_os_error_a0 *error;
    struct ce_os_object_a0 *object;
    struct ce_os_path_a0 *path;
    struct ce_os_process_a0 *process;
    struct ce_os_thread_a0 *thread;
    struct ce_os_time_a0 *time;
    struct ce_os_vio_a0 *vio;
    struct ce_os_window_a0 *window;
    struct ce_os_input_a0 *input;
};

CE_MODULE(ce_os_a0);


#endif //CE_OS_H
