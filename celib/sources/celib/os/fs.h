#ifndef CETECH_FS_H
#define CETECH_FS_H

#include "../memory/memory.h"
#include "../containers/array.h"

#include <time.h>

//==============================================================================
// File Interface
//==============================================================================

//! Get file modified time
//! \param path File path
//! \return Modified time
time_t os_file_mtime(const char *path);

//==============================================================================
// Path Interface
//==============================================================================

//! List dir
//! \param path Dir path
//! \param recursive Resucrsive list?
//! \param files Result files
//! \param allocator Allocator
void os_dir_list(const char *path,
                 int recursive,
                 struct array_pchar *files,
                 struct allocator *allocator);

//! Free list dir array
//! \param files Files array
//! \param allocator Allocator
void os_dir_list_free(struct array_pchar *files,
                      struct allocator *allocator);

//! Create dir
//! \param path Dir path
//! \return 1 of ok else 0
int os_dir_make(const char *path);

//! Create dir path
//! \param path Path
//! \return 1 of ok else 0
int os_dir_make_path(const char *path);


#endif //CETECH_FS_H
