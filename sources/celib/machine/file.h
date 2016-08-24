#ifndef CETECH_FILE_H
#define CETECH_FILE_H

//==============================================================================
// Includes
//==============================================================================

#include <time.h>
#include <stdint.h>
#include "celib/types.h"

#include "types.h"

//==============================================================================
// File Interface
//==============================================================================

//! Get file modified time
//! \param path File path
//! \return Modified time
time_t file_mtime(const char* path);

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

#endif //CETECH_FILE_H
