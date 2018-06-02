#ifndef CETECH_VIO_H
#define CETECH_VIO_H



//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <stddef.h>

//==============================================================================
// VIO
//==============================================================================

enum ct_vio_open_mode {
    VIO_OPEN_READ,
    VIO_OPEN_WRITE,
};

enum ct_vio_seek {
    VIO_SEEK_SET,
    VIO_SEEK_CUR,
    VIO_SEEK_END
};

typedef void ct_vio_instance_t;

struct ct_vio {
    ct_vio_instance_t *inst;

    int64_t (*size)(struct ct_vio *vio);

    int64_t (*seek)(struct ct_vio *vio,
                    int64_t offset,
                    enum ct_vio_seek whence);

    size_t (*read)(struct ct_vio *vio,
                   void *ptr,
                   size_t size,
                   size_t maxnum);

    size_t (*write)(struct ct_vio *vio,
                    const void *ptr,
                    size_t size,
                    size_t num);

    int (*close)(struct ct_vio *vio);
};

struct ct_vio_a0 {
    struct ct_vio *(*from_file)(const char *path,
                                enum ct_vio_open_mode mode);
};


#endif //CETECH_VIO_H
