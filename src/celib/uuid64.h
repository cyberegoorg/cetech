#ifndef CETECH_UUID64_H
#define CETECH_UUID64_H

#include <fcntl.h>
#include <unistd.h>

#include "celib_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ce_uuid64_t0 {
    uint64_t u;
} ce_uuid64_t0;

static inline ce_uuid64_t0 ce_uuid64_generate() {
    ce_uuid64_t0 uuid;
    int fd = open("/dev/urandom", O_RDONLY);
    read(fd, &uuid, sizeof(uuid));
    close(fd);
    return uuid;
}

static inline bool ce_uuid64_try_parse(ce_uuid64_t0 *guid,
                                       const char *str) {
    CE_ASSERT("UUID", NULL != str);

    int num = sscanf(str, "0x%llx", &guid->u);
    return num == 1;
}

static inline void ce_uuid64_to_string(char *buf,
                                       uint32_t len,
                                       const ce_uuid64_t0 *guid) {
    snprintf(buf, len, "0x%llx", guid->u);
}

#ifdef __cplusplus
};
#endif

#endif //CETECH_UUID64_H
