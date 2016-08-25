#ifndef CETECH_VIO_TYPES_H
#define CETECH_VIO_TYPES_H

//==============================================================================
// Enums
//==============================================================================

#include <stdio.h>
#include <celib/types.h>

enum open_mode {
    VIO_OPEN_READ,
    VIO_OPEN_WRITE,
};

enum vio_seek {
    VIO_SEEK_SET,
    VIO_SEEK_CUR,
    VIO_SEEK_END
};


//==============================================================================
// Structs
//==============================================================================

struct vio {
    i64 (*size)(struct vio *vio);

    i64 (*seek)(struct vio *vio, i64 offset, enum vio_seek whence);

    size_t (*read)(struct vio *vio, void *ptr, size_t size, size_t maxnum);

    size_t (*write)(struct vio *vio, const void *ptr, size_t size, size_t num);

    int (*close)(struct vio *vio);
};

#endif //CETECH_VIO_TYPES_H
