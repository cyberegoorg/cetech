//==============================================================================
// Includes
//==============================================================================
#include <stdio.h>

#include <cetech/celib/allocator.h>
#include <cetech/core/yaml.h>

#include <cetech/core/application.h>
#include <cetech/core/memory.h>
#include <cetech/core/config.h>
#include <cetech/core/module.h>
#include <cetech/core/hash.h>
#include <cetech/core/log.h>
#include <cetech/core/path.h>
#include <cetech/core/vio.h>
#include <cetech/core/api.h>

#include <cetech/modules/resource.h>
#include <cstring>

#include "_config.h"

//==============================================================================
// Defines
//==============================================================================

#define MAX_VARIABLES 1024
#define MAX_NAME_LEN 128
#define MAX_DESC_LEN 256
#define LOG_WHERE "cvar"

#define make_cvar(i) (cvar_t){.idx = i}

#define str_set(result, str) memcpy(result, str, strlen(str))

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
    uint64_t type;
} _G;

IMPORT_API(memory_api_v0);
IMPORT_API(path_v0);
IMPORT_API(vio_api_v0);
IMPORT_API(log_api_v0);
IMPORT_API(hash_api_v0);

//==============================================================================
// Privates
//==============================================================================


void _dealloc_allm_string() {
    for (int i = 0; i < MAX_VARIABLES; ++i) {
        if (_G.types[i] != CV_STRING) {
            continue;
        }

        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), _G.values[i].s);
    }
}

cvar_t _find_first_free() {

    for (uint64_t i = 1; i < MAX_VARIABLES; ++i) {
        if (_G.name[i][0] != '\0') {
            continue;
        }

        return make_cvar(i);
    }

    log_api_v0.error(LOG_WHERE, "Could not create _new config variable");

    return make_cvar(0);
}


//==============================================================================
// Interface
//==============================================================================
namespace config {

#ifdef CETECH_CAN_COMPILE

    void compile_global(struct app_api_v0 *app_api) {
        char build_dir[1024] = {0};
        char source_path[1024] = {0};
        char build_path[1024] = {0};

        cvar_t bd = find("build");
        cvar_t source_dir = find("src");

        const char *build_dir_str = get_string(bd);
        path_v0.join(build_dir, 1024, build_dir_str, app_api->platform());

        path_v0.join(build_path, CETECH_ARRAY_LEN(build_path), build_dir,
                     "global.config");


        path_v0.join(source_path, CETECH_ARRAY_LEN(source_path),
                     get_string(source_dir), "global.config");

        struct vio *source_vio = vio_api_v0.from_file(source_path,
                                                      VIO_OPEN_READ,
                                                      memory_api_v0.main_allocator());
        char *data =
                CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                                vio_api_v0.size(source_vio));

        size_t size = (size_t) vio_api_v0.size(source_vio);
        vio_api_v0.read(source_vio, data, sizeof(char), size);
        vio_api_v0.close(source_vio);

        struct vio *build_vio = vio_api_v0.from_file(build_path, VIO_OPEN_WRITE,
                                                     memory_api_v0.main_allocator());
        vio_api_v0.write(build_vio, data, sizeof(char), size);
        vio_api_v0.close(build_vio);

        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), data);
    }

#endif

    cvar_t find(const char *name) {
        for (uint64_t i = 1; i < MAX_VARIABLES; ++i) {
            if (_G.name[i][0] == '\0') {
                continue;
            }

            if (strcmp(_G.name[i], name) != 0) {
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
        struct foreach_config_data *output = (foreach_config_data *) _data;

        char key_str[128] = {0};
        yaml_as_string(key, key_str, CETECH_ARRAY_LEN(key_str));

        char name[1024] = {0};
        if (output->root_name != NULL) {
            snprintf(name, CETECH_ARRAY_LEN(name), "%s.%s", output->root_name,
                     key_str);
        } else {
            snprintf(name, CETECH_ARRAY_LEN(name), "%s", key_str);
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

            cvar_t cvar = find(name);
            if (cvar.idx != 0) {
                enum cvar_type t = get_type(cvar);
                switch (t) {
                    case CV_NONE:
                        break;
                    case CV_FLOAT:
                        tmp_f = yaml_as_float(value);
                        set_float(cvar, tmp_f);
                        break;
                    case CV_INT:
                        tmp_int = yaml_as_int(value);
                        set_int(cvar, tmp_int);
                        break;
                    case CV_STRING:
                        yaml_as_string(value, tmp_str,
                                       CETECH_ARRAY_LEN(tmp_str));
                        set_string(cvar, tmp_str);
                        break;
                }
            }
        }
    }


    void cvar_load_global(struct app_api_v0 *app_api) {
        char build_dir[1024] = {0};
        char source_path[1024] = {0};

        cvar_t bd = find("build");
        cvar_t source_dir = find("src");

        const char *build_dir_str = get_string(bd);
        path_v0.join(build_dir, 1024, build_dir_str, app_api->platform());


        path_v0.join(source_path, CETECH_ARRAY_LEN(source_path), build_dir,
                     "global.config");

        struct vio *source_vio = vio_api_v0.from_file(source_path,
                                                      VIO_OPEN_READ,
                                                      memory_api_v0.main_allocator());
        char *data =
                CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                                vio_api_v0.size(source_vio));
        vio_api_v0.read(source_vio, data, vio_api_v0.size(source_vio),
                        vio_api_v0.size(source_vio));
        vio_api_v0.close(source_vio);

        yaml_document_t h;
        yaml_node_t root = yaml_load_str(data, &h);
        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), data);

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

        cvar_t cvar = find(name);
        if (cvar.idx != 0) {
            enum cvar_type type = _G.types[cvar.idx];
            switch (type) {
                case CV_FLOAT:
                    sscanf(value, "%f", &tmp_var.f);
                    set_float(cvar, tmp_var.f);
                    break;

                case CV_INT:
                    if (value == NULL) {
                        tmp_var.i = 1;
                    } else {
                        sscanf(value, "%d", &tmp_var.i);
                    }

                    set_int(cvar, tmp_var.i);
                    break;

                case CV_STRING:
                    set_string(cvar, value);
                    break;

                default:
                    log_api_v0.error(LOG_WHERE, "Invalid type for cvar \"%s\"",
                                     name);
                    break;
            }

        } else {
            log_api_v0.error(LOG_WHERE, "Invalid cvar \"%s\"", name);
        }
    }

    struct args {
        int argc;
        const char **argv;
    };

    int parse_args(int argc,
                   const char **argv) {
        struct args tmp_args = {.argc = argc, .argv = argv};
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

    int parse_core_args(int argc,
                        const char **argv) {
        struct args tmp_args = {.argc = argc, .argv = argv};
        for (int j = 0; j < tmp_args.argc; ++j) {
            if (tmp_args.argv[j][0] != '-') {
                continue;
            }

            const char *name = tmp_args.argv[j] + 1;

            if (!strcmp(name, "build") ||
                !strcmp(name, "compile") ||
                !strcmp(name, "src")) {

                const char *value = (j != tmp_args.argc - 1) ? tmp_args.argv[j +
                                                                             1]
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


    cvar_t find_or_create(const char *name,
                          int *_new) {
        if (_new) *_new = 0;

        for (uint64_t i = 1; i < MAX_VARIABLES; ++i) {
            if (_G.name[i][0] == '\0') {
                continue;
            }

            if (strcmp(_G.name[i], name) != 0) {
                continue;
            }

            return make_cvar(i);
        }

        const cvar_t var = _find_first_free();

        if (var.idx != 0) {
            str_set(_G.name[var.idx], name);

            if (_new) *_new = 1;
            return var;
        }

        return make_cvar(0);
    }

    cvar_t new_float(const char *name,
                     const char *desc,
                     float f) {
        int _new;
        cvar_t find = find_or_create(name, &_new);

        if (_new) {
            str_set(_G.name[find.idx], name);
            _G.types[find.idx] = CV_FLOAT;
            _G.values[find.idx].f = f;
        }

        str_set(_G.desc[find.idx], desc);

        return find;
    }

    cvar_t new_int(const char *name,
                   const char *desc,
                   int i) {
        int _new;
        cvar_t find = find_or_create(name, &_new);

        if (_new) {
            str_set(_G.name[find.idx], name);
            _G.types[find.idx] = CV_INT;
            _G.values[find.idx].i = i;
        }

        str_set(_G.desc[find.idx], desc);

        return find;
    }

    cvar_t new_str(const char *name,
                   const char *desc,
                   const char *s) {
        int _new;
        cvar_t find = find_or_create(name, &_new);

        if (_new) {
            str_set(_G.name[find.idx], name);
            _G.types[find.idx] = CV_STRING;
            _G.values[find.idx].s = memory_api_v0.str_dup(s,
                                                          memory_api_v0.main_allocator());
        }

        str_set(_G.desc[find.idx], desc);

        return find;
    }

    float get_float(cvar_t var) {
        return _G.values[var.idx].f;
    }

    int get_int(cvar_t var) {
        return _G.values[var.idx].i;
    }

    const char *get_string(cvar_t var) {
        return _G.values[var.idx].s;
    }

    enum cvar_type get_type(cvar_t var) {
        return _G.types[var.idx];
    }

    void set_float(cvar_t var,
                   float f) {
        _G.values[var.idx].f = f;
    }

    void set_int(cvar_t var,
                 int i) {
        _G.values[var.idx].i = i;
    }

    void set_string(cvar_t var,
                    const char *s) {
        char *_s = _G.values[var.idx].s;

        if (_s != NULL) {
            allocator_deallocate(memory_api_v0.main_allocator(), _s);
        }

        _G.values[var.idx].s = memory_api_v0.str_dup(s,
                                                     memory_api_v0.main_allocator());
    }

    void log_all() {
        for (uint64_t i = 1; i < MAX_VARIABLES; ++i) {
            if (_G.name[i][0] == '\0') {
                continue;
            }

            const char *name = _G.name[i];

            switch (_G.types[i]) {
                case CV_FLOAT:
                    log_api_v0.info(LOG_WHERE, "%s = %f", name, _G.values[i].f);
                    break;
                case CV_INT:
                    log_api_v0.info(LOG_WHERE, "%s = %d", name, _G.values[i].i);
                    break;
                case CV_STRING:
                    log_api_v0.info(LOG_WHERE, "%s = %s", name, _G.values[i].s);
                    break;
                default:
                    break;
            }
        }
    }

    int init(struct api_v0 *api) {

        GET_API(api, memory_api_v0);
        GET_API(api, path_v0);
        GET_API(api, vio_api_v0);
        GET_API(api, log_api_v0);
        GET_API(api, hash_api_v0);

        _G = {0};

        log_api_v0.debug(LOG_WHERE, "Init");

        static struct config_api_v0 api_v1 = {
                .load_global = cvar_load_global,
#ifdef CETECH_CAN_COMPILE
                .compile_global = compile_global,
#endif
                .parse_core_args = parse_core_args,
                .parse_args = parse_args,
                .find = find,
                .find_or_create = find_or_create,
                .new_float = new_float,
                .new_int = new_int,
                .new_str = new_str,
                .get_float = get_float,
                .get_int = get_int,
                .get_string = get_string,
                .get_type  = get_type,
                .set_float = set_float,
                .set_int = set_int,
                .set_string = set_string,
                .log_all = log_all,
        };

        api->register_api("config_api_v0", &api_v1);

        _G.type = hash_api_v0.id64_from_str("config");

        return 1;
    }

    void shutdown() {
        log_api_v0.debug(LOG_WHERE, "Shutdown");

        _dealloc_allm_string();
    }

};