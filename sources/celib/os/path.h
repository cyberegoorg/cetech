#ifndef CETECH_PATH_H
#define CETECH_PATH_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <celib/types.h>

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


#endif //CETECH_PATH_H
