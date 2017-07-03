

#include <cetech/celib/array.inl>

#include <cetech/kernel/api.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/blob.h>


using namespace cetech;

typedef cetech::Array<uint8_t> blob_array;

namespace blob {
    uint8_t *data(blob_instance_v0 *inst) {
        blob_array *array = (blob_array *) inst;
        return array::begin(*array);
    }

    uint64_t size(blob_instance_v0 *inst) {
        blob_array *array = (blob_array *) inst;
        return array::size(*array);
    }

    void push(blob_instance_v0 *inst,
              void *data,
              uint64_t size) {
        blob_array *array = (blob_array *) inst;
        return array::push(*array, (const uint8_t *) data, size);
    }

    blob_v0 *create(allocator *allocator) {
        blob_array *inst = CETECH_NEW(allocator,
                                      blob_array,
                                      allocator);

        blob_v0 *blob = CETECH_NEW(allocator,
                                   blob_v0);
        blob->inst = inst;
        blob->push = push;
        blob->size = size;
        blob->data = data;

        return blob;
    }

    void destroy(blob_v0 *blob) {
        blob_array *inst = (blob_array *) blob->inst;
        allocator *a = inst->_allocator;

        CETECH_DELETE(a, blob_array, inst);
        CETECH_DELETE(a, blob_v0, blob);
    }

    static blob_api_v0 api{
            .create = blob::create,
            .destroy = blob::destroy,
    };
}


namespace blob_module {
    static void _init(struct api_v0 *api) {
        api->register_api("blob_api_v0", &blob::api);
    }

    extern "C" void *blob_load_module(struct api_v0 *api) {
        _init(api);
        return nullptr;
    }

    extern "C" void blob_unload_module(struct api_v0 *api) {
    }
}