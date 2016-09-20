#ifndef CETECH_PATH_H
#define CETECH_PATH_H

#include <stdio.h>
#include "../types.h"

//! Get filename from path
//! \param path Path
//! \return Filename
const char *os_path_filename(const char *path);

//! Get file basename (filename without extension)
//! \param path Path
//! \param out Out basename
//! \param size
void os_path_basename(const char *path,
                      char *out,
                      size_t size);

void os_path_dir(char *out,
                 size_t size,
                 const char *path);

//! Get file extension
//! \param path Path
//! \return file extension
const char *os_path_extension(const char *path);

//! Join path
//! \param result Output path
//! \param maxlen Result len
//! \param base_path Base path
//! \param path Path
//! \return Result path len
i64 os_path_join(char *result,
                 u64 maxlen,
                 const char *base_path,
                 const char *path);


#endif //CETECH_PATH_H
