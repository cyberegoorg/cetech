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

typedef struct ce_vio_o0 ce_vio_o0;

typedef struct ce_vio_t0 {
    ce_vio_o0 *inst;

    int64_t (*size)(ce_vio_o0 *inst);

    int64_t (*seek)(ce_vio_o0 *inst,
                    int64_t offset,
                    enum ce_vio_seek whence);

    size_t (*read)(ce_vio_o0 *inst,
                   void *ptr,
                   size_t size,
                   size_t maxnum);

    size_t (*write)(ce_vio_o0 *inst,
                    const void *ptr,
                    size_t size,
                    size_t num);

    int (*close)(ce_vio_o0 *inst);
} ce_vio_t0;

struct ce_os_vio_a0 {
    struct ce_vio_t0 *(*from_file)(const char *path,
                                   enum ce_vio_open_mode mode);

    void (*close)(struct ce_vio_t0 * vio);
};


CE_MODULE(ce_os_vio_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_OS_VIO_H
