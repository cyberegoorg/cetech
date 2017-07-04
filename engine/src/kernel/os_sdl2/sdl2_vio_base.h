#include <stddef.h>

#include <cetech/kernel/os.h>
#include <cetech/celib/allocator.h>
#include <cetech/kernel/errors.h>
#include <cetech/kernel/os.h>

struct os_vio *vio_from_file(const char *path,
                             enum vio_open_mode mode,
                             struct allocator *allocator);

