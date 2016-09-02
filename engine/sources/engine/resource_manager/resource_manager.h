#ifndef CETECH_RESOURCE_MANAGER_H
#define CETECH_RESOURCE_MANAGER_H

#include "celib/memory/types.h"
#include "celib/os/vio_types.h"

typedef void *(*resource_loader_t)(struct vio *input, struct allocator *allocator);

typedef void  (*resource_online_t)(void *data);

typedef void  (*resource_offline_t)(void *data);

typedef void  (*resource_unloader_t)(void *new_data, struct allocator *allocator);

typedef void *(*resource_reloader_t)(stringid64_t name, void *old_data, void *new_data, struct allocator *allocator);

typedef struct {
    resource_loader_t loader;
    resource_unloader_t unloader;
    resource_online_t online;
    resource_offline_t offline;
    resource_reloader_t reloader;
} resource_callbacks_t;

int resource_init();

void resource_shutdown();

void resource_set_autoload(int enable);

void resource_register_type(stringid64_t type,
                            resource_callbacks_t callbacks);

void resource_load(void **loaded_data, stringid64_t type, stringid64_t *names, size_t count);

void resource_add_loaded(stringid64_t type, stringid64_t *names, void **resource_data, size_t count);

void resource_load_now(stringid64_t type, stringid64_t *names, size_t count);

void resorucemanager_unload(stringid64_t type, stringid64_t *names, size_t count);

void resource_reload(stringid64_t type, stringid64_t *names, size_t count);

int resource_can_get(stringid64_t type, stringid64_t names);

int resource_can_get_all(stringid64_t type, stringid64_t *names, size_t count);

void *resource_get(stringid64_t type, stringid64_t names);


#endif //CETECH_RESOURCE_MANAGER_H
