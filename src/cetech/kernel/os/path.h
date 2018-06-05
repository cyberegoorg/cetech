#ifndef CETECH_PATH_H
#define CETECH_PATH_H




//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct ct_alloc;

//==============================================================================
// Path
//==============================================================================

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
                 const char *patern,
                 int recursive,
                 int only_dir,
                 char ***files,
                 uint32_t *count,
                 struct ct_alloc *allocator);

    void (*list2)(const char *path,
                  const char *patern,
                  int recursive,
                  int only_dir,
                  void(*on_item)(const char *filename));

    //! Free list dir array
    //! \param files Files array
    //! \param allocator Allocator
    void (*list_free)(char **files,
                      uint32_t count,
                      struct ct_alloc *allocator);

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
                     char *out);

    void (*dir)(char *out,
                const char *path);

    void (*dirname)(char *out,
                    const char *path);

    //! Get file extension
    //! \param path Path
    //! \return file extension
    const char *(*extension)(const char *path);

    //! Join paths
    //! \param allocator Allocator
    //! \param count Path count.
    //! \return Result path len.
    void (*join)(char** buffer,
                  struct ct_alloc *allocator,
                  uint32_t count,
                  ...);

    void (*copy_file)(struct ct_alloc *allocator,
                      const char *from,
                      const char *to);

    bool (*is_dir)(const char *path);
};


#endif //CETECH_PATH_H
