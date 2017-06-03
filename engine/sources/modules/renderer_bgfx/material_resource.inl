#ifndef CETECH_MATERIAL_RESORCE_H
#define CETECH_MATERIAL_RESORCE_H

#include <cetech/core/path.h>

namespace material_resource {

    static const bgfx_program_handle_t null_program = {0};

    void *loader(struct vio *input,
                 struct allocator *allocator) {
        const int64_t size = vio_api_v0.size(input);
        char *data = CETECH_ALLOCATE(allocator, char, size);
        vio_api_v0.read(input, data, 1, size);

        return data;
    }

    void unloader(void *new_data,
                  struct allocator *allocator) {
        CETECH_DEALLOCATE(allocator, new_data);
    }

    void online(uint64_t name,
                void *data) {
    }

    void offline(uint64_t name,
                 void *data) {
    }

    void *reloader(uint64_t name,
                   void *old_data,
                   void *new_data,
                   struct allocator *allocator) {
        offline(name, old_data);
        online(name, new_data);

        CETECH_DEALLOCATE(allocator, old_data);
        return new_data;
    }

}

#endif //CETECH_MATERIAL_RESORCE_H
