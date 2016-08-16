#ifndef CETECH_OS_TYPES_H
#define CETECH_OS_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include "celib/types.h"


//==============================================================================
// Enums
//==============================================================================

enum open_mode {
    VFILE_OPEN_READ,
    VFILE_OPEN_WRITE,
};

enum vfile_seek {
    VFILE_SEEK_SET,
    VFILE_SEEK_CUR,
    VFILE_SEEK_END
};


//==============================================================================
// Structs
//==============================================================================

struct vfile {
    i64 (*size)(struct vfile *vfile);
    i64 (*seek)(struct vfile *vfile, i64 offset, enum vfile_seek whence);
    size_t (*read)(struct vfile *vfile, void *ptr, size_t size, size_t maxnum);
    size_t (*write)(struct vfile *vfile, const void *ptr, size_t size, size_t num);
    int (*close)(struct vfile *vfile);
};

#endif //CETECH_OS_TYPES_H
