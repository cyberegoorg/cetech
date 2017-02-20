//==============================================================================
// Includes
//==============================================================================

#include "include/SDL2/SDL.h"
#include "include/mpack/mpack.h"

#include "celib/containers/map.h"
#include <celib/filesystem/vio.h>
#include <celib/filesystem/path.h>

#include "engine/resource/types.h"
#include <engine/config/cvar.h>
#include <engine/application/application.h>
#include <engine/develop/console_server.h>
#include <engine/memory/memsys.h>
#include <engine/plugin/plugin_api.h>
#include <engine/plugin/plugin.h>
#include <engine/filesystem/types.h>

#include "../types.h"
#include "engine/memory/memsys.h"


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
    int autoload_enabled;

    struct {
        cvar_t build_dir;
    } config;

} _G = {0};

static struct MemSysApiV1 MemSysApiV1;
static struct ConsoleServerApiV1 ConsoleServerApiV1;
static struct FilesystemApiV1 FilesystemApiV1;
static struct ConfigApiV1 ConfigApiV1;
static struct ApplicationApiV1 ApplicationApiV1;

void resource_set_autoload(int enable);

void resource_register_type(stringid64_t type,
                            resource_callbacks_t callbacks);

void resource_load(void **loaded_data,
                   stringid64_t type,
                   stringid64_t *names,
                   size_t count,
                   int force);

void resource_add_loaded(stringid64_t type,
                         stringid64_t *names,
                         void **resource_data,
                         size_t count);

void resource_load_now(stringid64_t type,
                       stringid64_t *names,
                       size_t count);

void resource_unload(stringid64_t type,
                     stringid64_t *names,
                     size_t count);

void resource_reload(stringid64_t type,
                     stringid64_t *names,
                     size_t count);

void resource_reload_all();

int resource_can_get(stringid64_t type,
                     stringid64_t names);

int resource_can_get_all(stringid64_t type,
                         stringid64_t *names,
                         size_t count);

void *resource_get(stringid64_t type,
                   stringid64_t names);

int resource_type_name_string(char *str,
                              size_t max_len,
                              stringid64_t type,
                              stringid64_t name);

void resource_compiler_register(stringid64_t type,
                                resource_compilator_t compilator);

void resource_compiler_compile_all();

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   stringid64_t type,
                                   stringid64_t name);

const char *resource_compiler_get_source_dir();

const char *resource_compiler_get_core_dir();

int resource_compiler_get_build_dir(char *build_dir,
                                    size_t max_len,
                                    const char *platform);

int resource_compiler_get_tmp_dir(char *tmp_dir,
                                  size_t max_len,
                                  const char *platform);

int resource_compiler_external_join(char *output,
                                    u32 max_len,
                                    const char *name);

void resource_compiler_create_build_dir();

void package_load(stringid64_t name);

void package_unload(stringid64_t name);

int package_is_loaded(stringid64_t name);

void package_flush(stringid64_t name);

//==============================================================================
// Private
//==============================================================================

void resource_reload_all();

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
                              struct cel_allocator *allocator) {
    const i64 size = cel_vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    cel_vio_read(input, data, 1, size);

    return data;
}

void package_resource_unloader(void *new_data,
                               struct cel_allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
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
                                struct cel_allocator *allocator) {
    CEL_DEALLOCATE(allocator, old_data);
    return new_data;
}

static const resource_callbacks_t package_resource_callback = {
        .loader = package_resource_loader,
        .unloader =package_resource_unloader,
        .online =package_resource_online,
        .offline =package_resource_offline,
        .reloader = package_resource_reloader
};

extern int package_init();

extern void package_shutdown();

void resource_register_type(stringid64_t type,
                            resource_callbacks_t callbacks);

static void _init(get_api_fce_t get_engine_api) {
    ConsoleServerApiV1 = *((struct ConsoleServerApiV1 *) get_engine_api(CONSOLE_SERVER_API_ID, 0));
    MemSysApiV1 = *(struct MemSysApiV1 *) get_engine_api(MEMORY_API_ID, 0);
    FilesystemApiV1 = *(struct FilesystemApiV1 *) get_engine_api(FILESYSTEM_API_ID, 0);
    ConfigApiV1 = *(struct ConfigApiV1 *) get_engine_api(CONFIG_API_ID, 0);
    ApplicationApiV1 = *(struct ApplicationApiV1 *) get_engine_api(APPLICATION_API_ID, 0);

    ARRAY_INIT(resource_data, &_G.resource_data, MemSysApiV1.main_allocator());
    ARRAY_INIT(resource_callbacks_t, &_G.resource_callbacks, MemSysApiV1.main_allocator());
    MAP_INIT(u32, &_G.type_map, MemSysApiV1.main_allocator());

    _G.config.build_dir = ConfigApiV1.find("build");


    char build_dir_full[4096] = {0};
    cel_path_join(build_dir_full,
                  CEL_ARRAY_LEN(build_dir_full),
                  ConfigApiV1.get_string(_G.config.build_dir),
                  ApplicationApiV1.platform());

    FilesystemApiV1.filesystem_map_root_dir(stringid64_from_string("build"), build_dir_full);

    resource_register_type(stringid64_from_string("package"), package_resource_callback);

    ConsoleServerApiV1.consolesrv_register_command("resource.reload_all", _cmd_reload_all);

    package_init();
    //return package_init();

}

static void _init_cvar(struct ConfigApiV1 config) {
    _G = (struct G) {0};
}

static void _shutdown() {
    for (int i = 0; i < ARRAY_SIZE(&_G.resource_data); ++i) {
        MAP_DESTROY(resource_item_t, &ARRAY_AT(&_G.resource_data, i));
    }

    ARRAY_DESTROY(resource_data, &_G.resource_data);
    ARRAY_DESTROY(resource_callbacks_t, &_G.resource_callbacks);
    MAP_DESTROY(u32, &_G.type_map);

    package_shutdown();
}



//==============================================================================
// Public interface
//==============================================================================

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

    MAP_INIT(resource_item_t, &ARRAY_AT(&_G.resource_data, idx), MemSysApiV1.main_allocator());

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

void resource_load(void **loaded_data,
                   stringid64_t type,
                   stringid64_t *names,
                   size_t count,
                   int force);

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
        resource_type_name_string(build_name, CEL_ARRAY_LEN(build_name), type, names[i]);

        char filename[4096] = {0};
        resource_compiler_get_filename(filename, CEL_ARRAY_LEN(filename), type, names[i]);
        log_debug("resource", "Loading resource %s from %s/%s", filename,
                  FilesystemApiV1.filesystem_get_root_dir(root_name),
                  build_name);

        struct vio *resource_file = FilesystemApiV1.filesystem_open(root_name, build_name, VIO_OPEN_READ);

        if (resource_file != NULL) {
            loaded_data[i] = type_clb.loader(resource_file, MemSysApiV1.main_allocator());
            FilesystemApiV1.filesystem_close(resource_file);
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
            resource_type_name_string(build_name, CEL_ARRAY_LEN(build_name), type, names[i]);

            char filename[1024] = {0};
            resource_compiler_get_filename(filename, CEL_ARRAY_LEN(filename), type, names[i]);
            log_debug("resource", "Unload resource %s ", filename);

            type_clb.offline(names[i], item.data);
            type_clb.unloader(item.data, MemSysApiV1.main_allocator());

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
        resource_type_name_string(build_name, CEL_ARRAY_LEN(build_name), type, names);

        if (_G.autoload_enabled) {
            char filename[1024] = {0};
            resource_compiler_get_filename(filename, CEL_ARRAY_LEN(filename), type, names);

            log_warning(LOG_WHERE, "Autoloading resource %s", filename);
            resource_load_now(type, &names, 1);
            item = MAP_GET(resource_item_t, resource_map, names.id, null_item);
        } else {
            // TODO: fallback resource #205
            CEL_ASSERT(LOG_WHERE, false);
        }
    }

    return item.data;
}

void resource_reload(stringid64_t type,
                     stringid64_t *names,
                     size_t count) {
    plugin_reload_all();

    void *loaded_data[count];
    MAP_T(resource_item_t) *resource_map = _get_resource_map(type);
    const u32 idx = MAP_GET(u32, &_G.type_map, type.id, 0);

    resource_callbacks_t type_clb = ARRAY_AT(&_G.resource_callbacks, idx);

    resource_load(loaded_data, type, names, count, 1);
    for (int i = 0; i < count; ++i) {
//        char build_name[33] = {0};
//        resource_type_name_string(build_name, CEL_ARRAY_LEN(build_name), type, names[i]);

        char filename[1024] = {0};
        resource_compiler_get_filename(filename, CEL_ARRAY_LEN(filename), type, names[i]);

        log_debug("resource", "Reload resource %s ", filename);

        void *old_data = resource_get(type, names[i]);

        void *new_data = type_clb.reloader(names[i], old_data, loaded_data[i], MemSysApiV1.main_allocator());

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
    ARRAY_INIT(stringid64_t, &name_array, MemSysApiV1.main_allocator());

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

void *resourcesystem_get_plugin_api(int api,
                                    int version) {
    switch (api) {
        case PLUGIN_EXPORT_API_ID:
            switch (version) {
                case 0: {
                    static struct plugin_api_v0 plugin = {0};

                    plugin.init = _init;
                    plugin.shutdown = _shutdown;
                    plugin.init_cvar = _init_cvar;

                    return &plugin;
                }

                default:
                    return NULL;
            };
        case RESOURCE_API_ID:
            switch (version) {
                case 0: {
                    static struct ResourceApiV1 api = {0};

                    api.set_autoload = resource_set_autoload;
                    api.register_type = resource_register_type;
                    api.load = resource_load;
                    api.add_loaded = resource_add_loaded;
                    api.load_now = resource_load_now;
                    api.unload = resource_unload;
                    api.reload = resource_reload;
                    api.reload_all = resource_reload_all;
                    api.can_get = resource_can_get;
                    api.can_get_all = resource_can_get_all;
                    api.get = resource_get;
                    api.type_name_string = resource_type_name_string;
                    api.compiler_register = resource_compiler_register;
                    api.compiler_compile_all = resource_compiler_compile_all;
                    api.compiler_get_filename = resource_compiler_get_filename;
                    api.compiler_get_build_dir = resource_compiler_get_build_dir;
                    api.compiler_get_tmp_dir = resource_compiler_get_tmp_dir;
                    api.compiler_external_join = resource_compiler_external_join;
                    api.compiler_create_build_dir = resource_compiler_create_build_dir;
                    api.compiler_get_core_dir = resource_compiler_get_core_dir;
                    api.compiler_get_source_dir = resource_compiler_get_source_dir;

                    return &api;
                }

                default:
                    return NULL;
            };

        case PACKAGE_API_ID:
            switch (version) {
                case 0: {
                    static struct PackageApiV1 api = {0};

                    api.load = package_load;
                    api.unload = package_unload;
                    api.is_loaded = package_is_loaded;
                    api.flush = package_flush;

                    return &api;
                }

                default:
                    return NULL;
            };

        default:
            return NULL;
    }

}