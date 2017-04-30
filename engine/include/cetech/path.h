#ifndef CELIB_PATH_H
#define CELIB_PATH_H

#include <stdint.h>
#include <stddef.h>

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

void path_dir(char *out,
                  size_t size,
                  const char *path);

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
int64_t path_join(char *result,
                  uint64_t maxlen,
                  const char *base_path,
                  const char *path);

#endif //CELIB_PATH_H
