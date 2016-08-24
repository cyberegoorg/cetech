#ifndef CETECH_MACHINE_H
#define CETECH_MACHINE_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/math/types.h"
#include "types.h"

typedef int (*machine_part_init_t)();

typedef void (*machine_part_shutdown_t)();

typedef void (*machine_part_process_t)(struct eventstream *stream);

//==============================================================================
// Machine interface
//==============================================================================

int machine_init();

void machine_shutdown();

void machine_register_part(const char *name,
                           machine_part_init_t init,
                           machine_part_shutdown_t shutdown,
                           machine_part_process_t process);

void machine_process();

struct event_header *machine_event_begin();

struct event_header *machine_event_end();

struct event_header *machine_event_next(struct event_header *header);

//==============================================================================
// CPU
// ==============================================================================

int machine_cpu_count();

//==============================================================================
// Time
// ==============================================================================

u32 machine_get_ticks();

//==============================================================================
// SO
//==============================================================================

void *machine_load_object(const char *path);

void machine_unload_object(void *so);

void *machine_load_function(void *so, void *name);

//==============================================================================
// Window interface
//==============================================================================

window_t machine_window_new(const char *title,
                            enum WindowPos x,
                            enum WindowPos y,
                            const i32 width,
                            const i32 height,
                            enum WindowFlags flags);

window_t machine_window_new_from(void *hndl);

void machine_window_destroy(window_t w);

void machine_window_set_title(window_t w,
                              const char *title);

const char *machine_window_get_title(window_t w);

void machine_window_update(window_t w);

void machine_window_resize(window_t w,
                           uint32_t width,
                           uint32_t height);

void *machine_window_native_window_ptr(window_t w);

void *machine_window_native_display_ptr(window_t w);

//==============================================================================
// File Interface
//==============================================================================

//! Get file modified time
//! \param path File path
//! \return Modified time
time_t file_mtime(const char *path);

//==============================================================================
// VFile Interface
//==============================================================================

struct vfile *vfile_from_file(const char *path,
                              enum open_mode mode,
                              struct allocator *allocator);

int vfile_close(struct vfile *file);

int64_t vfile_seek(struct vfile *file,
                   int64_t offset,
                   enum vfile_seek whence);

void vfile_seek_to_end(struct vfile *file);

i64 vfile_skip(struct vfile *, i64 bytes);

i64 vfile_position(struct vfile *file);

i64 vfile_size(struct vfile *file);

size_t vfile_read(struct vfile *file,
                  void *buffer,
                  size_t size,
                  size_t maxnum);

size_t vfile_write(struct vfile *file,
                   const void *buffer,
                   size_t size,
                   size_t num);

//==============================================================================
// Interface
//==============================================================================

void *os_malloc(size_t size);

void os_free(void *ptr);


//==============================================================================
// Path Interface
//==============================================================================

//! Get filename from path
//! \param path Path
//! \return Filename
const char *path_filename(const char *path);

//! Get file basename (filename without extension)
//! \param path Path
//! \param out Out basename
//! \param size
void path_basename(const char *path,
                   char *out,
                   size_t size);

//! Get file extension
//! \param path Path
//! \return file extension
const char *path_extension(const char *path);

//! Join path
//! \param result Output path
//! \param maxlen Result len
//! \param base_path Base path
//! \param path Path
//! \return Result path len
i64 path_join(char *result, u64 maxlen, const char *base_path, const char *path);

//! List dir
//! \param path Dir path
//! \param recursive Resucrsive list?
//! \param files Result files
//! \param allocator Allocator
void path_listdir(const char *path,
                  int recursive,
                  struct array_pchar *files,
                  struct allocator *allocator);

//! Free list dir array
//! \param files Files array
//! \param allocator Allocator
void path_listdir_free(struct array_pchar *files, struct allocator *allocator);

//! Create dir
//! \param path Dir path
//! \return 1 of ok else 0
int path_mkdir(const char *path);

//! Create dir path
//! \param path Path
//! \return 1 of ok else 0
int path_mkpath(const char *path);


//==============================================================================
// Thread
//==============================================================================


//! Create new thread
//! \param fce Thread fce
//! \param name Thread name
//! \param data Thread data
//! \return new thread
thread_t thread_create(thread_fce_t fce,
                       const char *name,
                       void *data);

//! Kill thread
//! \param thread thread
void thread_kill(thread_t thread);

//! Wait for thread
//! \param thread Thread
//! \param status Thread exit status
void thread_wait(thread_t thread,
                 int *status);

//! Get id for thread
//! \param thread Thread
//! \return ID
u64 thread_get_id(thread_t thread);


//! Get actual thread id
//! \return Thread id
u64 thread_actual_id();


void spin_lock(spinlock_t *lock);

void spin_unlock(spinlock_t *lock);

#endif //CETECH_MACHINE_H
