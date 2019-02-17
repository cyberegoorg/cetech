//                          **OS vio**
//

#ifndef CE_OS_VIO_H
#define CE_OS_VIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#define CE_OS_VIO_API \
    CE_ID64_0("ce_os_vio_a0", 0x989fe40200d61012ULL)


enum ce_vio_open_mode {
    VIO_OPEN_READ,
    VIO_OPEN_WRITE,
};

enum ce_vio_seek {
    VIO_SEEK_SET = 1,
    VIO_SEEK_CUR,
    VIO_SEEK_END
};

typedef void ce_vio_instance_t;

typedef struct ce_vio_t0 {
    ce_vio_instance_t *inst;

    int64_t (*size)(struct ce_vio_t0 *vio);

    int64_t (*seek)(struct ce_vio_t0 *vio,
                    int64_t offset,
                    enum ce_vio_seek whence);

    size_t (*read)(struct ce_vio_t0 *vio,
                   void *ptr,
                   size_t size,
                   size_t maxnum);

    size_t (*write)(struct ce_vio_t0 *vio,
                    const void *ptr,
                    size_t size,
                    size_t num);

    int (*close)(struct ce_vio_t0 *vio);
} ce_vio_t0;

struct ce_os_vio_a0 {
    struct ce_vio_t0 *(*from_file)(const char *path,
                                   enum ce_vio_open_mode mode);
};


CE_MODULE(ce_os_vio_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_OS_VIO_H
