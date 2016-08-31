//==============================================================================
// Includes
//==============================================================================

#include "include/SDL2/SDL.h"

#include <celib/stringid/types.h>
#include <engine/resource_manager/resource_manager.h>
#include <celib/stringid/stringid.h>
#include <engine/filesystem/filesystem.h>
#include <engine/config_system/config_system.h>
#include <celib/os/path.h>
#include <engine/application/application.h>
#include "engine/memory_system/memory_system.h"
#include "celib/containers/hash.h"

//==============================================================================
// Struct and types
//==============================================================================

typedef struct {
    void *data;
    u8 ref_count;
} resource_item_t;

ARRAY_PROTOTYPE(resource_item_t)

MAP_PROTOTYPE(resource_item_t)

ARRAY_PROTOTYPE_N(MAP_T(resource_item_t), resource_data)

ARRAY_PROTOTYPE(resource_callbacks_t)


//==============================================================================
// Gloals
//==============================================================================


static const resource_item_t default_item = {.data=NULL, .ref_count=0};

#define _G ResourceManagerGlobals
struct G {
    MAP_T(u32) type_map;
    ARRAY_T(resource_data) resource_data;
    ARRAY_T(resource_callbacks_t) resource_callbacks;
    config_var_t cv_build_dir;
} _G = {0};


//==============================================================================
// Private
//==============================================================================

static MAP_T(resource_item_t) *_get_resource_map(stringid64_t type) {
    const u32 idx = MAP_GET(u32, &_G.type_map, type.id, 0);

    return &ARRAY_AT(&_G.resource_data, idx);
}

//==============================================================================
// Public interface
//==============================================================================

int resource_init() {
    _G = (struct G) {0};

    ARRAY_INIT(resource_data, &_G.resource_data, memsys_main_allocator());
    ARRAY_INIT(resource_callbacks_t, &_G.resource_callbacks, memsys_main_allocator());
    MAP_INIT(u32, &_G.type_map, memsys_main_allocator());

    _G.cv_build_dir = config_find("resource_compiler.build_dir");

    char build_dir_full[1024] = {0};
    os_path_join(build_dir_full,
                 CE_ARRAY_LEN(build_dir_full),
                 config_get_string(_G.cv_build_dir),
                 application_platform());

    filesystem_map_root_dir(stringid64_from_string("build"), build_dir_full);

    return 1;
}

void resource_shutdown() {

    for (int i = 0; i < ARRAY_SIZE(&_G.resource_data); ++i) {
        MAP_DESTROY(resource_item_t, &ARRAY_AT(&_G.resource_data, i));
    }

    ARRAY_DESTROY(resource_data, &_G.resource_data);
    ARRAY_DESTROY(resource_callbacks_t, &_G.resource_callbacks);
    MAP_DESTROY(u32, &_G.type_map);

    _G = (struct G) {0};
}

void resource_register_type(stringid64_t type,
                            resource_callbacks_t callbacks) {

    const u32 idx = ARRAY_SIZE(&_G.resource_data);

    ARRAY_PUSH_BACK(resource_data, &_G.resource_data, (MAP_T(resource_item_t)) {0});
    ARRAY_PUSH_BACK(resource_callbacks_t, &_G.resource_callbacks, callbacks);

    MAP_INIT(resource_item_t, &ARRAY_AT(&_G.resource_data, idx), memsys_main_allocator());
    MAP_SET(u32, &_G.type_map, type.id, idx);
}

void resource_add_loaded(stringid64_t type, stringid64_t *names, void **resource_data, size_t count) {
    const u32 idx = MAP_GET(u32, &_G.type_map, type.id, 0);

    MAP_T(resource_item_t) *resource_map = &ARRAY_AT(&_G.resource_data, idx);

    resource_item_t item = {.ref_count=1};
    for (size_t i = 0; i < count; i++) {
        if (resource_data[i] == 0) {
            continue;
        }

        item.data = resource_data[i];
        MAP_SET(resource_item_t, resource_map, names[i].id, item);

        ARRAY_AT(&_G.resource_callbacks, idx).online(resource_data[i]);
    }
}

void resource_load_now(stringid64_t type, stringid64_t *names, size_t count) {
    void *loaded_data[count];

    resource_load(loaded_data, type, names, count);
    resource_add_loaded(type, names, loaded_data, count);
}

int resource_can_get(stringid64_t type, stringid64_t names) {
    MAP_T(resource_item_t) *resource_map = _get_resource_map(type);

    return MAP_HAS(resource_item_t, resource_map, names.id);
}

int resource_can_get_all(stringid64_t type, stringid64_t *names, size_t count) {
    MAP_T(resource_item_t) *resource_map = _get_resource_map(type);

    for (size_t i = 0; i < count; ++i) {
        if (!MAP_HAS(resource_item_t, resource_map, names[i].id)) {
            return 0;
        }
    }

    return 1;
}

static int type_name_to_str(char *result, size_t maxlen, stringid64_t type, stringid64_t name) {
    return snprintf(result, maxlen, "%" SDL_PRIX64 "%" SDL_PRIX64, type.id, name.id);
}

void resource_load(void **loaded_data, stringid64_t type, stringid64_t *names, size_t count) {
    const u32 idx = MAP_GET(u32, &_G.type_map, type.id, 0);
    const stringid64_t root_name = stringid64_from_string("build");

    MAP_T(resource_item_t) *resource_map = &ARRAY_AT(&_G.resource_data, idx);

    resource_callbacks_t type_clb = ARRAY_AT(&_G.resource_callbacks, idx);


    for (int i = 0; i < count; ++i) {
        resource_item_t item = MAP_GET(resource_item_t, resource_map, names[i].id, default_item);

        if (item.ref_count > 0) {
            ++item.ref_count;
            MAP_SET(resource_item_t, resource_map, names[i].id, item);
            loaded_data[i] = 0;
            continue;
        }

        char build_name[33] = {0};
        type_name_to_str(build_name, CE_ARRAY_LEN(build_name), type, names[i]);
        log_debug("resource_manager", "Loading resource %s from %s", build_name, filesystem_get_root_dir(root_name));

        struct vio *resource_file = filesystem_open(root_name, build_name, VIO_OPEN_READ);

        if (resource_file != NULL) {
            loaded_data[i] = type_clb.loader(resource_file, memsys_main_allocator());
        } else {
            loaded_data[i] = 0;
        }

        filesystem_close(resource_file);
    }
}

void resorucemanager_unload(stringid64_t type, stringid64_t *names, size_t count) {
    const u32 idx = MAP_GET(u32, &_G.type_map, type.id, 0);
    MAP_T(resource_item_t) *resource_map = _get_resource_map(type);

    resource_callbacks_t type_clb = ARRAY_AT(&_G.resource_callbacks, idx);

    for (int i = 0; i < count; ++i) {
        resource_item_t item = MAP_GET(resource_item_t, resource_map, names[i].id, default_item);

        if (item.ref_count == 0) {
            continue;
        }

        if (--item.ref_count == 0) {
            char build_name[33] = {0};
            type_name_to_str(build_name, CE_ARRAY_LEN(build_name), type, names[i]);
            log_debug("resource_manager", "Unload resource %s ", build_name);

            type_clb.offline(item.data);
            type_clb.unloader(item.data, memsys_main_allocator());
        }

        MAP_SET(resource_item_t, resource_map, names[i].id, item);
    }
}

void *resource_get(stringid64_t type, stringid64_t names) {
    MAP_T(resource_item_t) *resource_map = _get_resource_map(type);

    return MAP_GET(resource_item_t, resource_map, names.id, default_item).data;
}

void resource_reload(stringid64_t type, stringid64_t *names, size_t count) {
    void *loaded_data[count];
    MAP_T(resource_item_t) *resource_map = _get_resource_map(type);
    const u32 idx = MAP_GET(u32, &_G.type_map, type.id, 0);

    resource_callbacks_t type_clb = ARRAY_AT(&_G.resource_callbacks, idx);

    resource_load(loaded_data, type, names, count);
    for (int i = 0; i < count; ++i) {
        char build_name[33] = {0};
        type_name_to_str(build_name, CE_ARRAY_LEN(build_name), type, names[i]);
        log_debug("resource_manager", "Reload resource %s ", build_name);


        void *old_data = resource_get(type, names[i]);

        type_clb.reloader(names[i], old_data, loaded_data[i], memsys_main_allocator());

        resource_item_t item = MAP_GET(resource_item_t, resource_map, names[i].id, default_item);
        item.data = loaded_data[i];
        --item.ref_count; // Load call increse item.ref_count, because is loaded
        MAP_SET(resource_item_t, resource_map, names[i].id, item);
    }
}