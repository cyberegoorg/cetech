//==============================================================================
// Includes
//==============================================================================

#include <celib/memory/memory.h>
#include <engine/config/api.h>
#include <celib/filesystem/vio.h>
#include <celib/filesystem/path.h>
#include <celib/yaml/yaml.h>
#include <engine/memory/api.h>
#include <engine/module/api.h>
#include <engine/module/module.h>
#include <celib/string/stringid.h>
#include "engine/resource/api.h"

//==============================================================================
// Defines
//==============================================================================

#define MAX_VARIABLES 1024
#define MAX_NAME_LEN 128
#define MAX_DESC_LEN 256
#define LOG_WHERE "cvar"

#define make_cvar(i) (cvar_t){.idx = i}

//==============================================================================
// Enums
//==============================================================================

static const char *_type_to_str[4] = {
        [CV_NONE] = "invalid",
        [CV_FLOAT] = "float",
        [CV_INT] = "int",
        [CV_STRING] = "string"
};

//==============================================================================
// Globals
//==============================================================================

#define _G ConfigSystemGlobals


static struct G {
    char name[MAX_VARIABLES][MAX_NAME_LEN];
    char desc[MAX_VARIABLES][MAX_DESC_LEN];

    enum cvar_type types[MAX_VARIABLES];

    union {
        float f;
        int i;
        char *s;
    } values[MAX_VARIABLES];
    stringid64_t type;
} _G = {0};

static struct MemSysApiV0 MemSysApiV0;

void cvar_load_global();

void cvar_compile_global();

int cvar_parse_core_args(struct args args);

int cvar_parse_args(struct args args);

cvar_t cvar_find(const char *name);

cvar_t cvar_find_or_create(const char *name,
                           int *new);

cvar_t cvar_new_float(const char *name,
                      const char *desc,
                      float f);

cvar_t cvar_new_int(const char *name,
                    const char *desc,
                    int i);

cvar_t cvar_new_str(const char *name,
                    const char *desc,
                    const char *s);

float cvar_get_float(cvar_t var);

int cvar_get_int(cvar_t var);

const char *cvar_get_string(cvar_t var);

enum cvar_type cvar_get_type(cvar_t var);

void cvar_set_float(cvar_t var,
                    float f);

void cvar_set_int(cvar_t var,
                  int i);

void cvar_set_string(cvar_t var,
                     const char *s);

void cvar_log_all();


//==============================================================================
// Privates
//==============================================================================


void _dealloc_allm_string() {
    for (int i = 0; i < MAX_VARIABLES; ++i) {
        if (_G.types[i] != CV_STRING) {
            continue;
        }

        CEL_DEALLOCATE(MemSysApiV0.main_allocator(), _G.values[i].s);
    }
}

cvar_t _find_first_free() {

    for (u64 i = 1; i < MAX_VARIABLES; ++i) {
        if (_G.name[i][0] != '\0') {
            continue;
        }

        return make_cvar(i);
    }

    log_error(LOG_WHERE, "Could not create new config variable");

    return make_cvar(0);
}

static void _init(get_api_fce_t get_engine_api) {

}

static void _shutdown() {
}

static void *_reload_begin(get_api_fce_t get_engine_api) {
    return NULL;
}

static void _reload_end(get_api_fce_t get_engine_api,
                        void *data) {
    _init(get_engine_api);
}




//==============================================================================
// Interface
//==============================================================================

int cvar_init() {
    log_debug(LOG_WHERE, "Init");

    MemSysApiV0 = *(struct MemSysApiV0 *) module_get_engine_api(MEMORY_API_ID,
                                                                0);

    _G.type = stringid64_from_string("config");

    return 1;
}

void cvar_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    _dealloc_allm_string();
}

void cvar_compile_global() {
    char build_dir[1024] = {0};
    char source_path[1024] = {0};
    char build_path[1024] = {0};

    struct ResourceApiV0 ResourceApiV0 = *(struct ResourceApiV0 *) module_get_engine_api(
            RESOURCE_API_ID, 0);
    struct ApplicationApiV0 ApplicationApiV0 = *(struct ApplicationApiV0 *) module_get_engine_api(
            APPLICATION_API_ID,
            0);

    ResourceApiV0.compiler_get_build_dir(build_dir, CEL_ARRAY_LEN(build_dir),
                                         ApplicationApiV0.platform());
    cel_path_join(build_path, CEL_ARRAY_LEN(build_path), build_dir,
                  "global.config");
    cel_path_join(source_path, CEL_ARRAY_LEN(source_path),
                  ResourceApiV0.compiler_get_source_dir(), "global.config");

    struct vio *source_vio = cel_vio_from_file(source_path, VIO_OPEN_READ,
                                               MemSysApiV0.main_allocator());
    char *data =
    CEL_ALLOCATE(MemSysApiV0.main_allocator(), char,
                 cel_vio_size(source_vio));

    size_t size = (size_t) cel_vio_size(source_vio);
    cel_vio_read(source_vio, data, sizeof(char), size);
    cel_vio_close(source_vio);

    struct vio *build_vio = cel_vio_from_file(build_path, VIO_OPEN_WRITE,
                                              MemSysApiV0.main_allocator());
    cel_vio_write(build_vio, data, sizeof(char), size);
    cel_vio_close(build_vio);

    CEL_DEALLOCATE(MemSysApiV0.main_allocator(), data);
}


cvar_t cvar_find(const char *name) {
    for (u64 i = 1; i < MAX_VARIABLES; ++i) {
        if (_G.name[i][0] == '\0') {
            continue;
        }

        if (cel_strcmp(_G.name[i], name) != 0) {
            continue;
        }

        return make_cvar(i);
    }

    return make_cvar(0);
}

struct foreach_config_data {
    char *root_name;
};

void foreach_config_clb(yaml_node_t key,
                        yaml_node_t value,
                        void *_data) {
    struct foreach_config_data *output = _data;

    char key_str[128] = {0};
    yaml_as_string(key, key_str, CEL_ARRAY_LEN(key_str));

    char name[1024] = {0};
    if (output->root_name != NULL) {
        snprintf(name, CEL_ARRAY_LEN(name), "%s.%s", output->root_name,
                 key_str);
    } else {
        snprintf(name, CEL_ARRAY_LEN(name), "%s", key_str);
    }

    enum yaml_node_type type = yaml_node_type(value);

    if (type == YAML_TYPE_MAP) {
        struct foreach_config_data _data = {
                .root_name = name
        };

        yaml_node_foreach_dict(value, foreach_config_clb, &_data);
    } else if (type == YAML_TYPE_SCALAR) {
        char value_str[128] = {0};

        float tmp_f;
        int tmp_int;
        char tmp_str[128];

        cvar_t cvar = cvar_find(name);
        if (cvar.idx != 0) {
            enum cvar_type t = cvar_get_type(cvar);
            switch (t) {
                case CV_NONE:
                    break;
                case CV_FLOAT:
                    tmp_f = yaml_as_float(value);
                    cvar_set_float(cvar, tmp_f);
                    break;
                case CV_INT:
                    tmp_int = yaml_as_int(value);
                    cvar_set_int(cvar, tmp_int);
                    break;
                case CV_STRING:
                    yaml_as_string(value, tmp_str, CEL_ARRAY_LEN(tmp_str));
                    cvar_set_string(cvar, tmp_str);
                    break;
            }
        }
    }
}


void cvar_load_global() {
    char build_dir[1024] = {0};
    char source_path[1024] = {0};

    struct ResourceApiV0 ResourceApiV0 = *(struct ResourceApiV0 *) module_get_engine_api(
            RESOURCE_API_ID, 0);
    struct ApplicationApiV0 ApplicationApiV0 = *(struct ApplicationApiV0 *) module_get_engine_api(
            APPLICATION_API_ID,
            0);
    ResourceApiV0.compiler_get_build_dir(build_dir, CEL_ARRAY_LEN(build_dir),
                                         ApplicationApiV0.platform());
    cel_path_join(source_path, CEL_ARRAY_LEN(source_path), build_dir,
                  "global.config");

    struct vio *source_vio = cel_vio_from_file(source_path, VIO_OPEN_READ,
                                               MemSysApiV0.main_allocator());
    char *data =
    CEL_ALLOCATE(MemSysApiV0.main_allocator(), char,
                 cel_vio_size(source_vio));
    cel_vio_read(source_vio, data, cel_vio_size(source_vio),
                 cel_vio_size(source_vio));
    cel_vio_close(source_vio);

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(data, &h);
    CEL_DEALLOCATE(MemSysApiV0.main_allocator(), data);

    struct foreach_config_data config_data = {
            .root_name = NULL
    };

    yaml_node_foreach_dict(root, foreach_config_clb, &config_data);
}

void _cvar_from_str(const char *name,
                    const char *value) {
    union {
        float f;
        int i;
        const char *s;
    } tmp_var;

    cvar_t cvar = cvar_find(name);
    if (cvar.idx != 0) {
        enum cvar_type type = _G.types[cvar.idx];
        switch (type) {
            case CV_FLOAT:
                sscanf(value, "%f", &tmp_var.f);
                cvar_set_float(cvar, tmp_var.f);
                break;

            case CV_INT:
                if (value == NULL) {
                    tmp_var.i = 1;
                } else {
                    sscanf(value, "%d", &tmp_var.i);
                }

                cvar_set_int(cvar, tmp_var.i);
                break;

            case CV_STRING:
                cvar_set_string(cvar, value);
                break;

            default:
                log_error(LOG_WHERE, "Invalid type for cvar \"%s\"", name);
                break;
        }

    } else {
        log_error(LOG_WHERE, "Invalid cvar \"%s\"", name);
    }
}

int cvar_parse_args(struct args args) {
    struct args tmp_args = args;
    for (int j = 0; j < tmp_args.argc; ++j) {
        if (tmp_args.argv[j][0] != '-') {
            continue;
        }

        const char *name = tmp_args.argv[j] + 1;
        const char *value = (j != tmp_args.argc - 1) ? tmp_args.argv[j + 1]
                                                     : NULL;

        if (value && (value[0] == '-')) {
            value = NULL;
        } else {
            ++j;
        }

        _cvar_from_str(name, value);
    }

    return 1;
}

int cvar_parse_core_args(struct args args) {
    struct args tmp_args = args;
    for (int j = 0; j < tmp_args.argc; ++j) {
        if (tmp_args.argv[j][0] != '-') {
            continue;
        }

        const char *name = tmp_args.argv[j] + 1;

        if (!cel_strcmp(name, "build") ||
            !cel_strcmp(name, "compile") ||
            !cel_strcmp(name, "src")) {

            const char *value = (j != tmp_args.argc - 1) ? tmp_args.argv[j + 1]
                                                         : NULL;

            if (value && (value[0] == '-')) {
                value = NULL;
            } else {
                ++j;
            }

            _cvar_from_str(name, value);
        }
    }

    return 1;
}


cvar_t cvar_find_or_create(const char *name,
                           int *new) {
    if (new) *new = 0;

    for (u64 i = 1; i < MAX_VARIABLES; ++i) {
        if (_G.name[i][0] == '\0') {
            continue;
        }

        if (cel_strcmp(_G.name[i], name) != 0) {
            continue;
        }

        return make_cvar(i);
    }

    const cvar_t var = _find_first_free();

    if (var.idx != 0) {
        cel_str_set(_G.name[var.idx], name);

        if (new) *new = 1;
        return var;
    }

    return make_cvar(0);
}

cvar_t cvar_new_float(const char *name,
                      const char *desc,
                      float f) {
    int new;
    cvar_t find = cvar_find_or_create(name, &new);

    if (new) {
        cel_str_set(_G.name[find.idx], name);
        _G.types[find.idx] = CV_FLOAT;
        _G.values[find.idx].f = f;
    }

    cel_str_set(_G.desc[find.idx], desc);

    return find;
}

cvar_t cvar_new_int(const char *name,
                    const char *desc,
                    int i) {
    int new;
    cvar_t find = cvar_find_or_create(name, &new);

    if (new) {
        cel_str_set(_G.name[find.idx], name);
        _G.types[find.idx] = CV_INT;
        _G.values[find.idx].i = i;
    }

    cel_str_set(_G.desc[find.idx], desc);

    return find;
}

cvar_t cvar_new_str(const char *name,
                    const char *desc,
                    const char *s) {
    int new;
    cvar_t find = cvar_find_or_create(name, &new);

    if (new) {
        cel_str_set(_G.name[find.idx], name);
        _G.types[find.idx] = CV_STRING;
        _G.values[find.idx].s = cel_strdup(s, MemSysApiV0.main_allocator());
    }

    cel_str_set(_G.desc[find.idx], desc);

    return find;
}

float cvar_get_float(cvar_t var) {
    return _G.values[var.idx].f;
}

int cvar_get_int(cvar_t var) {
    return _G.values[var.idx].i;
}

const char *cvar_get_string(cvar_t var) {
    return _G.values[var.idx].s;
}

enum cvar_type cvar_get_type(cvar_t var) {
    return _G.types[var.idx];
}

void cvar_set_float(cvar_t var,
                    float f) {
    _G.values[var.idx].f = f;
}

void cvar_set_int(cvar_t var,
                  int i) {
    _G.values[var.idx].i = i;
}

void cvar_set_string(cvar_t var,
                     const char *s) {
    char *_s = _G.values[var.idx].s;

    if (_s != NULL) {
        allocator_deallocate(MemSysApiV0.main_allocator(), _s);
    }

    _G.values[var.idx].s = cel_strdup(s, MemSysApiV0.main_allocator());
}

void cvar_log_all() {
    for (u64 i = 1; i < MAX_VARIABLES; ++i) {
        if (_G.name[i][0] == '\0') {
            continue;
        }

        const char *name = _G.name[i];

        switch (_G.types[i]) {
            case CV_FLOAT:
                log_info(LOG_WHERE, "%s = %f", name, _G.values[i].f);
                break;
            case CV_INT:
                log_info(LOG_WHERE, "%s = %d", name, _G.values[i].i);
                break;
            case CV_STRING:
                log_info(LOG_WHERE, "%s = %s", name, _G.values[i].s);
                break;
            default:
                break;
        }
    }
}

void *config_get_module_api(int api,
                            int version) {

    if (api == PLUGIN_EXPORT_API_ID && version == 0) {
        static struct module_api_v0 module = {0};

        module.init = _init;
        module.shutdown = _shutdown;
        module.reload_begin = _reload_begin;
        module.reload_end = _reload_end;

        return &module;
    } else if (api == CONFIG_API_ID && version == 0) {
        static struct ConfigApiV0 api_v1 = {
                .load_global = cvar_load_global,
                .compile_global = cvar_compile_global,
                .parse_core_args = cvar_parse_core_args,
                .parse_args = cvar_parse_args,
                .find = cvar_find,
                .find_or_create = cvar_find_or_create,
                .new_float = cvar_new_float,
                .new_int = cvar_new_int,
                .new_str = cvar_new_str,
                .get_float = cvar_get_float,
                .get_int = cvar_get_int,
                .get_string = cvar_get_string,
                .get_type  = cvar_get_type,
                .set_float = cvar_set_float,
                .set_int = cvar_set_int,
                .set_string = cvar_set_string,
                .log_all = cvar_log_all,
        };

        return &api_v1;
    }

    return 0;
}