#ifndef CELIB_FS_H
#define CELIB_FS_H

#include <time.h>

#include "memory.h"
#include "celib/array.h"

//==============================================================================
// File Interface
//==============================================================================

//! Get file modified time
//! \param path File path
//! \return Modified time
time_t cel_file_mtime(const char *path);

//==============================================================================
// Path Interface
//==============================================================================

//! List dir
//! \param path Dir path
//! \param recursive Resucrsive list?
//! \param files Result files
//! \param allocator Allocator
void cel_dir_list(const char *path,
                         int recursive,
                         struct array_pchar *files,
                         struct cel_allocator *allocator);

//! Free list dir array
//! \param files Files array
//! \param allocator Allocator
void cel_dir_list_free(struct array_pchar *files,
                              struct cel_allocator *allocator);

//! Create dir
//! \param path Dir path
//! \return 1 of ok else 0
int cel_dir_make(const char *path);

//! Create dir path
//! \param path Path
//! \return 1 of ok else 0
int cel_dir_make_path(const char *path);


#endif //CELIB_FS_H
