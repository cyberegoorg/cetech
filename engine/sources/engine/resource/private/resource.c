//==============================================================================
// Includes
//==============================================================================

#include "include/SDL2/SDL.h"

#include <engine/resource/resource.h>
#include <engine/resource/filesystem.h>
#include <celib/config/cvar.h>
#include <celib/filesystem/path.h>
#include <engine/application/application.h>
#include <celib/filesystem/vio.h>
#include <mpack/mpack.h>
#include <engine/develop/console_server.h>
#include "celib/containers/map.h"
#include <celib/memory/memsys.h>

//==============================================================================
// Struct and types
//==============================================================================

typedef struct {
    void *data;
    u8 ref_count;
} resource_item_t;

ARRAY_PROTOTYPE(resource_item_t)

ARRAY_PROTOTYPE(stringid64_t)

ARRAY_PROTOTYPE_N(const char*, cstring)

MAP_PROTOTYPE(resource_item_t)

ARRAY_PROTOTYPE_N(MAP_T(resource_item_t), resource_data)

ARRAY_PROTOTYPE(resource_callbacks_t)


//==============================================================================
// Gloals
//==============================================================================

#define LOG_WHERE "resource_manager"

//==============================================================================
// Gloals
//==============================================================================

static const resource_item_t null_item = {.data=NULL, .ref_count=0};

#define is_item_null(item) (item.data == null_item.data)

#define _G ResourceManagerGlobals
struct G {
    MAP_T(u32) type_map;
    ARRAY_T(resource_data) resource_data;
    ARRAY_T(resource_callbacks_t) resource_callbacks;
    cvar_t cv_build_dir;
    int autoload_enabled;
} _G = {0};


//==============================================================================
// Private
//==============================================================================

static int _cmd_reload_all(mpack_node_t args,
                           mpack_writer_t *writer) {
    resource_reload_all();
    return 0;
}

static MAP_T(resource_item_t) *_get_resource_map(stringid64_t type) {
    const u32 idx = MAP_GET(u32, &_G.type_map, type.id, UINT32_MAX);

    if (idx == UINT32_MAX) {
        return NULL;
    }

    return &ARRAY_AT(&_G.resource_data, idx);
}

void *package_resource_loader(struct vio *input,
                              struct allocator *allocator) {
    const i64 size = vio_size(input);
    char *data = CE_ALLOCATE(allocator, char, size);
    vio_read(input, data, 1, size);

    return data;
}

void package_resource_unloader(void *new_data,
                               struct allocator *allocator) {
    CE_DEALLOCATE(allocator, new_data);
}

void package_resource_online(stringid64_t name,
                             void *data) {
}

void package_resource_offline(stringid64_t name,
                              void *data) {
}

void *package_resource_reloader(stringid64_t name,
                                void *old_data,
                                void *new_data,
                                struct allocator *allocator) {
    CE_DEALLOCATE(allocator, old_data);
    return new_data;
}

static const resource_callbacks_t package_resource_callback = {
        .loader = package_resource_loader,
        .unloader =package_resource_unloader,
        .online =package_resource_online,
        .offline =package_resource_offline,
        .reloader = package_resource_reloader
};


//==============================================================================
// Public interface
//==============================================================================
extern int package_init();

extern void package_shutdown();


int resource_init(int stage) {
    if (stage == 0) {
        _G = (struct G) {0};

        ARRAY_INIT(resource_data, &_G.resource_data, memsys_main_allocator());
        ARRAY_INIT(resource_callbacks_t, &_G.resource_callbacks, memsys_main_allocator());
        MAP_INIT(u32, &_G.type_map, memsys_main_allocator());

        return 1;
    }

    _G.cv_build_dir = cvar_find("build");

    char build_dir_full[4096] = {0};
    celib_path_join(build_dir_full,
                    CE_ARRAY_LEN(build_dir_full),
                    cvar_get_string(_G.cv_build_dir),
                    application_platform());

    filesystem_map_root_dir(stringid64_from_string("build"), build_dir_full);

    resource_register_type(stringid64_from_string("package"), package_resource_callback);

    consolesrv_register_command("resource.reload_all", _cmd_reload_all);

    return package_init();
}

void resource_shutdown() {

    for (int i = 0; i < ARRAY_SIZE(&_G.resource_data); ++i) {
        MAP_DESTROY(resource_item_t, &ARRAY_AT(&_G.resource_data, i));
    }

    ARRAY_DESTROY(resource_data, &_G.resource_data);
    ARRAY_DESTROY(resource_callbacks_t, &_G.resource_callbacks);
    MAP_DESTROY(u32, &_G.type_map);

    package_shutdown();

    _G = (struct G) {0};
}

int resource_type_name_string(char *str,
                              size_t max_len,
                              stringid64_t type,
                              stringid64_t name) {
    return snprintf(str, max_len, "%" SDL_PRIX64 "%" SDL_PRIX64, type.id, name.id);
}


void resource_set_autoload(int enable) {
    _G.autoload_enabled = enable;
}

void resource_register_type(stringid64_t type,
                            resource_callbacks_t callbacks) {

    const u32 idx = ARRAY_SIZE(&_G.resource_data);

    ARRAY_PUSH_BACK(resource_data, &_G.resource_data, (MAP_T(resource_item_t)) {0});
    ARRAY_PUSH_BACK(resource_callbacks_t, &_G.resource_callbacks, callbacks);

    MAP_INIT(resource_item_t, &ARRAY_AT(&_G.resource_data, idx), memsys_main_allocator());

    MAP_SET(u32, &_G.type_map, type.id, idx);
}

void resource_add_loaded(stringid64_t type,
                         stringid64_t *names,
                         void **resource_data,
                         size_t count) {
    const u32 idx = MAP_GET(u32, &_G.type_map, type.id, UINT32_MAX);

    if (idx == UINT32_MAX) {
        return;
    }

    MAP_T(resource_item_t) *resource_map = &ARRAY_AT(&_G.resource_data, idx);

    resource_item_t item = {.ref_count=1};
    for (size_t i = 0; i < count; i++) {
        item.data = resource_data[i];
        MAP_SET(resource_item_t, resource_map, names[i].id, item);

        if (resource_data[i] == 0) {
            continue;
        }
        ARRAY_AT(&_G.resource_callbacks, idx).online(names[i], resource_data[i]);
    }
}

void resource_load_now(stringid64_t type,
                       stringid64_t *names,
                       size_t count) {
    void *loaded_data[count];

    resource_load(loaded_data, type, names, count, 0);
    resource_add_loaded(type, names, loaded_data, count);
}

int resource_can_get(stringid64_t type,
                     stringid64_t names) {
    MAP_T(resource_item_t) *resource_map = _get_resource_map(type);

    if (resource_map == NULL) {
        return 1;
    }

    return MAP_HAS(resource_item_t, resource_map, names.id);
}

int resource_can_get_all(stringid64_t type,
                         stringid64_t *names,
                         size_t count) {
    MAP_T(resource_item_t) *resource_map = _get_resource_map(type);

    if (resource_map == NULL) {
        return 1;
    }

    for (size_t i = 0; i < count; ++i) {
        if (!MAP_HAS(resource_item_t, resource_map, names[i].id)) {
            return 0;
        }
    }

    return 1;
}

void resource_load(void **loaded_data,
                   stringid64_t type,
                   stringid64_t *names,
                   size_t count,
                   int force) {
    const u32 idx = MAP_GET(u32, &_G.type_map, type.id, UINT32_MAX);

    if (idx == UINT32_MAX) {
        log_error(LOG_WHERE, "Loader for resource is not is not registred");
        memory_set(loaded_data, sizeof(void *), count);
        return;
    }

    const stringid64_t root_name = stringid64_from_string("build");

    MAP_T(resource_item_t) *resource_map = &ARRAY_AT(&_G.resource_data, idx);

    resource_callbacks_t type_clb = ARRAY_AT(&_G.resource_callbacks, idx);


    for (int i = 0; i < count; ++i) {
        resource_item_t item = MAP_GET(resource_item_t, resource_map, names[i].id, null_item);

        if (!force && (item.ref_count > 0)) {
            ++item.ref_count;
            MAP_SET(resource_item_t, resource_map, names[i].id, item);
            loaded_data[i] = 0;
            continue;
        }

        char build_name[33] = {0};
        resource_type_name_string(build_name, CE_ARRAY_LEN(build_name), type, names[i]);

        char filename[4096] = {0};
        resource_compiler_get_filename(filename, CE_ARRAY_LEN(filename), type, names[i]);
        log_debug("resource", "Loading resource %s from %s/%s", filename, filesystem_get_root_dir(root_name),
                  build_name);

        struct vio *resource_file = filesystem_open(root_name, build_name, VIO_OPEN_READ);

        if (resource_file != NULL) {
            loaded_data[i] = type_clb.loader(resource_file, memsys_main_allocator());
            filesystem_close(resource_file);
        } else {
            loaded_data[i] = 0;
        }
    }
}

void resource_unload(stringid64_t type,
                     stringid64_t *names,
                     size_t count) {
    const u32 idx = MAP_GET(u32, &_G.type_map, type.id, UINT32_MAX);

    if (idx == UINT32_MAX) {
        return;
    }

    MAP_T(resource_item_t) *resource_map = _get_resource_map(type);

    resource_callbacks_t type_clb = ARRAY_AT(&_G.resource_callbacks, idx);

    for (int i = 0; i < count; ++i) {
        resource_item_t item = MAP_GET(resource_item_t, resource_map, names[i].id, null_item);

        if (item.ref_count == 0) {
            continue;
        }

        if (--item.ref_count == 0) {
            char build_name[33] = {0};
            resource_type_name_string(build_name, CE_ARRAY_LEN(build_name), type, names[i]);

            char filename[1024] = {0};
            resource_compiler_get_filename(filename, CE_ARRAY_LEN(filename), type, names[i]);
            log_debug("resource", "Unload resource %s ", filename);

            type_clb.offline(names[i], item.data);
            type_clb.unloader(item.data, memsys_main_allocator());

            MAP_REMOVE(resource_item_t, resource_map, names[i].id);
        }

        MAP_SET(resource_item_t, resource_map, names[i].id, item);
    }
}

void *resource_get(stringid64_t type,
                   stringid64_t names) {
    MAP_T(resource_item_t) *resource_map = _get_resource_map(type);

    resource_item_t item = MAP_GET(resource_item_t, resource_map, names.id, null_item);
    if (is_item_null(item)) {
        char build_name[33] = {0};
        resource_type_name_string(build_name, CE_ARRAY_LEN(build_name), type, names);

        if (_G.autoload_enabled) {
            char filename[1024] = {0};
            resource_compiler_get_filename(filename, CE_ARRAY_LEN(filename), type, names);

            log_warning(LOG_WHERE, "Autoloading resource %s", filename);
            resource_load_now(type, &names, 1);
            item = MAP_GET(resource_item_t, resource_map, names.id, null_item);
        } else {
            // TODO: fallback resource #205
            CE_ASSERT(LOG_WHERE, false);
        }
    }

    return item.data;
}

void resource_reload(stringid64_t type,
                     stringid64_t *names,
                     size_t count) {
    void *loaded_data[count];
    MAP_T(resource_item_t) *resource_map = _get_resource_map(type);
    const u32 idx = MAP_GET(u32, &_G.type_map, type.id, 0);

    resource_callbacks_t type_clb = ARRAY_AT(&_G.resource_callbacks, idx);

    resource_load(loaded_data, type, names, count, 1);
    for (int i = 0; i < count; ++i) {
//        char build_name[33] = {0};
//        resource_type_name_string(build_name, CE_ARRAY_LEN(build_name), type, names[i]);

        char filename[1024] = {0};
        resource_compiler_get_filename(filename, CE_ARRAY_LEN(filename), type, names[i]);

        log_debug("resource", "Reload resource %s ", filename);

        void *old_data = resource_get(type, names[i]);

        void *new_data = type_clb.reloader(names[i], old_data, loaded_data[i], memsys_main_allocator());

        resource_item_t item = MAP_GET(resource_item_t, resource_map, names[i].id, null_item);
        item.data = new_data;
        //--item.ref_count; // Load call increase item.ref_count, because is loaded
        MAP_SET(resource_item_t, resource_map, names[i].id, item);
    }
}

void resource_reload_all() {
    const MAP_ENTRY_T(u32) *type_it = MAP_BEGIN(u32, &_G.type_map);
    const MAP_ENTRY_T(u32) *type_end = MAP_END(u32, &_G.type_map);

    ARRAY_T(stringid64_t) name_array = {0};
    ARRAY_INIT(stringid64_t, &name_array, memsys_main_allocator());

    while (type_it != type_end) {
        stringid64_t type_id = {.id = type_it->key};

        MAP_T(resource_item_t) *resource_map = _get_resource_map(type_id);

        const MAP_ENTRY_T(resource_item_t) *name_it = MAP_BEGIN(resource_item_t, resource_map);
        const MAP_ENTRY_T(resource_item_t) *name_end = MAP_END(resource_item_t, resource_map);

        ARRAY_RESIZE(stringid64_t, &name_array, 0);
        while (name_it != name_end) {
            stringid64_t name_id = {.id = name_it->key};

            ARRAY_PUSH_BACK(stringid64_t, &name_array, name_id);

            ++name_it;
        }

        resource_reload(type_id, &ARRAY_AT(&name_array, 0), ARRAY_SIZE(&name_array));

        ++type_it;
    }

    ARRAY_DESTROY(stringid64_t, &name_array);
}
