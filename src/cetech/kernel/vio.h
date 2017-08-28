#ifndef CETECH_VIO_H
#define CETECH_VIO_H

#ifdef __cplusplus
extern "C" {
#endif

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

    int64_t (*size)(ct_vio_instance_t *vio);

    int64_t (*seek)(ct_vio_instance_t *vio,
                    int64_t offset,
                    enum ct_vio_seek whence);

    size_t (*read)(ct_vio_instance_t *vio,
                   void *ptr,
                   size_t size,
                   size_t maxnum);

    size_t (*write)(ct_vio_instance_t *vio,
                    const void *ptr,
                    size_t size,
                    size_t num);

    int (*close)(ct_vio_instance_t *vio);
};

struct ct_vio_a0 {
    struct ct_vio *(*from_file)(const char *path,
                                enum ct_vio_open_mode mode);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_VIO_H
