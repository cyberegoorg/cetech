#ifndef CELIB_VIO_TYPES_H
#define CELIB_VIO_TYPES_H

#include <stdio.h>
#include "../types.h"

//==============================================================================
// Enums
//==============================================================================


enum open_mode {
    VIO_OPEN_READ,
    VIO_OPEN_WRITE,
};

enum cel_vio_seek {
    VIO_SEEK_SET,
    VIO_SEEK_CUR,
    VIO_SEEK_END
};


//==============================================================================
// Structs
//==============================================================================

struct vio {
    i64 (*size)(struct vio *vio);

    i64 (*seek)(struct vio *vio,
                i64 offset,
                enum cel_vio_seek whence);

    size_t (*read)(struct vio *vio,
                   void *ptr,
                   size_t size,
                   size_t maxnum);

    size_t (*write)(struct vio *vio,
                    const void *ptr,
                    size_t size,
                    size_t num);

    int (*close)(struct vio *vio);
};

#endif //CELIB_VIO_TYPES_H
