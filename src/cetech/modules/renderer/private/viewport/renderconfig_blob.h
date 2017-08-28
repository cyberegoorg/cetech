#ifndef CETECH_RENDERCONFIG_BLOB_H
#define CETECH_RENDERCONFIG_BLOB_H

#include <stdint.h>

namespace renderconfig_blob {

    typedef struct {
        uint32_t blob_version;

        uint32_t global_resource_count;
        uint32_t local_resource_count;
        uint32_t layer_count;
        uint32_t layer_entry_count;
        uint32_t viewport_count;
        // render_resource_t global_resource[global_resource_count];
        // render_resource_t local_resource[local_resource_count];
        // uint64_t layers_name[layer_count];
        // uint32_t layers_entry_count[layer_count];
        // uint32_t layers_entry_offset[layer_count];
        // uint32_t layers_localresource_count[layer_count];
        // uint32_t layers_localresource_offset[layer_count];
        // uint32_t entry_data_offset[layer_entry_count];
        // layer_entry_t layers_entry[layer_entry_count];
        // viewport_entry_t viewport[viewport_count];
        // uint8_t data[*];
    } blob_t;

    inline const blob_t *get(void *data) {
        return (blob_t *) (data);
    }

    inline const uint32_t blob_size(const blob_t *blob) {
        return (
                sizeof(blob_t) +
                (sizeof(render_resource_t) * blob->global_resource_count) +
                (sizeof(render_resource_t) * blob->local_resource_count) +
                (sizeof(uint64_t) * blob->layer_count) +
                (sizeof(uint32_t) * blob->layer_count) +
                (sizeof(uint32_t) * blob->layer_count) +
                (sizeof(uint32_t) * blob->layer_count) +
                (sizeof(uint32_t) * blob->layer_count) +
                (sizeof(uint32_t) * blob->layer_entry_count) +
                (sizeof(layer_entry_t) * blob->layer_entry_count) +
                (sizeof(viewport_entry_t) * blob->viewport_count) +
                0);
    }

    inline uint32_t global_resource_count(const blob_t *blob) {
        return blob->global_resource_count;
    }

    inline uint32_t local_resource_count(const blob_t *blob) {
        return blob->local_resource_count;
    }

    inline uint32_t layer_count(const blob_t *blob) {
        return blob->layer_count;
    }

    inline uint32_t layer_entry_count(const blob_t *blob) {
        return blob->layer_entry_count;
    }

    inline uint32_t viewport_count(const blob_t *blob) {
        return blob->viewport_count;
    }

    inline render_resource_t *global_resource(const blob_t *blob) {
        return (render_resource_t *) ((blob) + 1);
    }

    inline render_resource_t *local_resource(const blob_t *blob) {
        return ((render_resource_t *) (global_resource(blob) +
                                       (blob->global_resource_count)));
    }

    inline uint64_t *layers_name(const blob_t *blob) {
        return ((uint64_t *) (local_resource(blob) +
                              (blob->local_resource_count)));
    }

    inline uint32_t *layers_entry_count(const blob_t *blob) {
        return ((uint32_t *) (layers_name(blob) + (blob->layer_count)));
    }

    inline uint32_t *layers_entry_offset(const blob_t *blob) {
        return ((uint32_t *) (layers_entry_count(blob) + (blob->layer_count)));
    }

    inline uint32_t *layers_localresource_count(const blob_t *blob) {
        return ((uint32_t *) (layers_entry_offset(blob) + (blob->layer_count)));
    }

    inline uint32_t *layers_localresource_offset(const blob_t *blob) {
        return ((uint32_t *) (layers_localresource_count(blob) +
                              (blob->layer_count)));
    }

    inline uint32_t *entry_data_offset(const blob_t *blob) {
        return ((uint32_t *) (layers_localresource_offset(blob) +
                              (blob->layer_count)));
    }

    inline layer_entry_t *layers_entry(const blob_t *blob) {
        return ((layer_entry_t *) (entry_data_offset(blob) +
                                   (blob->layer_entry_count)));
    }

    inline viewport_entry_t *viewport(const blob_t *blob) {
        return ((viewport_entry_t *) (layers_entry(blob) +
                                      (blob->layer_entry_count)));
    }

    inline uint8_t *data(const blob_t *blob) {
        return ((uint8_t *) (viewport(blob) + (blob->viewport_count)));
    }

}

#endif // CETECH_RENDERCONFIG_BLOB_H