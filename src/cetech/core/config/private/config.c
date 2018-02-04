//==============================================================================
// Includes
//==============================================================================
#include <stdio.h>
#include <string.h>

#include <cetech/core/api/api_system.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/os/path.h>
#include <cetech/core/os/vio.h>
#include <cetech/core/log/log.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/config/config.h>
#include <cetech/core/module/module.h>
#include <cetech/core/yaml/yamlng.h>
#include <cetech/core/cdb/cdb.h>
#include <cetech/core/macros.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_cdb_a0);

//==============================================================================
// Defines
//==============================================================================

#define MAX_VARIABLES 1024
#define MAX_NAME_LEN 128
#define MAX_DESC_LEN 256
#define LOG_WHERE "cvar"

#define make_cvar(i) (ct_cvar){.idx = (i)}

#define str_set(result, str) memcpy(result, str, strlen(str))

//==============================================================================
// Enums
//==============================================================================

//static const char *_type_to_str[4] = {
//        [CV_NONE] = "invalid",
//        [CV_FLOAT] = "float",
//        [CV_INT] = "int",
//        [CV_STRING] = "string"
//};


//==============================================================================
// Globals
//==============================================================================

#define _G ConfigSystemGlobals

static struct ConfigSystemGlobals {
    uint64_t type;

    struct ct_cdb_obj_t *config_object;
    struct ct_cdb_obj_t *config_desc;
} _G;


//==============================================================================
// Privates
//==============================================================================


//==============================================================================
// Interface
//==============================================================================

static void log_all() {
}

static void _cvar_from_str(const char *name,
                           const char *value) {
    int d = 0;
    float f = 0;

    struct ct_cdb_writer_t *writer = ct_cdb_a0.write_begin(
            _G.config_object);

    const uint64_t key = CT_ID64_0(name);

    if (value == NULL) {
        ct_cdb_a0.set_uint32(writer, key, 1);
        goto end;
    }


    if (sscanf(value, "%d", &d)) {
        ct_cdb_a0.set_uint32(writer, key, d);
        goto end;

    } else if (sscanf(value, "%f", &f)) {
        ct_cdb_a0.set_float(writer, key, d);
        goto end;
    }

    ct_cdb_a0.set_string(writer, key, value);

    end:
    ct_cdb_a0.write_commit(writer);
}

struct foreach_config_data {
    char *root_name;
};


static void foreach_config_clb(struct ct_yamlng_node key,
                               struct ct_yamlng_node value,
                               void *_data) {

    struct foreach_config_data *output = (struct foreach_config_data *) _data;
    struct ct_yng_doc *d = key.d;

    const char *key_str = d->as_string(d->inst, key, "");

    char name[1024] = {};
    if (output->root_name != NULL) {
        snprintf(name, CETECH_ARRAY_LEN(name),
                 "%s.%s", output->root_name, key_str);
    } else {
        snprintf(name, CETECH_ARRAY_LEN(name), "%s", key_str);
    }

    enum node_type type = d->type(d->inst, value);

    if (type == NODE_MAP) {
        struct foreach_config_data data = {
                .root_name = name
        };

        d->foreach_dict_node(d->inst, value, foreach_config_clb, &data);

    } else if (type != NODE_SEQ) {
        float tmp_f;
        int tmp_int;
        const char *str;

        if (type == NODE_STRING) {
            str = d->as_string(d->inst, value, "");
            _cvar_from_str(name, str);

        } else {
            const uint64_t key = CT_ID64_0(name);

            if (ct_cdb_a0.prop_exist(_G.config_object, key)) {
                enum ct_cdb_prop_type t = ct_cdb_a0.prop_type(
                        _G.config_object, key);
                struct ct_cdb_writer_t *writer = ct_cdb_a0.write_begin(
                        _G.config_object);

                switch (t) {
                    case CDB_TYPE_NONE:
                        break;

                    case CDB_TYPE_FLOAT:
                        tmp_f = d->as_float(d->inst, value, 0.0f);
                        ct_cdb_a0.set_float(writer, key, tmp_f);
                        break;

                    case CDB_TYPE_UINT32:
                        tmp_int = (int) d->as_float(d->inst, value, 0.0f);
                        ct_cdb_a0.set_uint32(writer, key, tmp_int);
                        break;

                    case CDB_TYPE_STRPTR:
                        str = d->as_string(d->inst, value, "");
                        ct_cdb_a0.set_string(writer, key, str);
                        break;
                    default:
                        break;
                }

                ct_cdb_a0.write_begin(_G.config_object);
            }
        }
    }
}


static int load_from_yaml_file(const char *yaml,
                               struct ct_alloc *alloc) {

    struct ct_vio *f = ct_vio_a0.from_file(yaml, VIO_OPEN_READ);
    struct ct_yng_doc *d = ct_yng_a0.from_vio(f, alloc);
    f->close(f);

    struct foreach_config_data config_data = {
            .root_name = NULL
    };


    d->foreach_dict_node(d->inst, d->get(d->inst, 0), foreach_config_clb,
                         &config_data);

    ct_yng_a0.destroy(d);

    return 1;
}


static int parse_args(int argc,
                      const char **argv) {
    for (int j = 0; j < argc; ++j) {
        if (argv[j][0] != '-') {
            continue;
        }

        const char *name = argv[j] + 1;
        const char *value = (j != argc - 1) ? argv[j + 1] : NULL;

        if (value && (value[0] == '-')) {
            value = NULL;
        } else {
            ++j;
        }

        _cvar_from_str(name, value);
    }

    return 1;
}


static struct ct_cdb_obj_t *config_object() {
    return _G.config_object;
}

static struct ct_config_a0 config_a0 = {
        .config_object = config_object,
        .parse_args = parse_args,
        .log_all = log_all,
        .load_from_yaml_file = load_from_yaml_file
};

CETECH_MODULE_DEF(
        config,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_cdb_a0);
        },
        {
            CT_UNUSED(reload);

            _G = (struct _G) {0};

            ct_log_a0.debug(LOG_WHERE, "Init");

            _G.config_object = ct_cdb_a0.create_object();
            _G.config_desc = ct_cdb_a0.create_object();

            api->register_api("ct_config_a0", &config_a0);

            _G.type = CT_ID64_0("config");
        },
        {
            CT_UNUSED(api, reload);

            ct_log_a0.debug(LOG_WHERE, "Shutdown");

        }
)
