//==============================================================================
// Includes
//==============================================================================
#include <stdio.h>
#include <string.h>

#include <corelib/api_system.h>
#include <corelib/memory.h>
#include <corelib/os.h>

#include <corelib/log.h>
#include <corelib/hashlib.h>
#include <corelib/config.h>
#include <corelib/module.h>
#include <corelib/yng.h>
#include <corelib/cdb.h>
#include <corelib/macros.h>


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
    uint32_t type;

    struct ct_cdb_t db;
    uint64_t config_object;
    uint64_t config_desc;
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

    ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(_G.config_object);

    const uint64_t key = ct_hashlib_a0->id64_from_str(name);

    if (value == NULL) {
        ct_cdb_a0->set_uint64(writer, key, 1);
        goto end;
    }


    if (sscanf(value, "%d", &d)) {
        ct_cdb_a0->set_uint64(writer, key, d);
        goto end;

    } else if (sscanf(value, "%f", &f)) {
        ct_cdb_a0->set_float(writer, key, d);
        goto end;
    }

    ct_cdb_a0->set_str(writer, key, value);

    end:
    ct_cdb_a0->write_commit(writer);
}

struct foreach_config_data {
    char *root_name;
};


static void foreach_config_clb(struct ct_yng_node key,
                               struct ct_yng_node value,
                               void *_data) {

    struct foreach_config_data *output = (struct foreach_config_data *) _data;
    struct ct_yng_doc *d = key.d;

    const char *key_str = d->as_string(d, key, "");

    char name[1024] = {};
    if (output->root_name != NULL) {
        snprintf(name, CT_ARRAY_LEN(name),
                 "%s.%s", output->root_name, key_str);
    } else {
        snprintf(name, CT_ARRAY_LEN(name), "%s", key_str);
    }

    enum node_type type = d->type(d, value);

    if (type == NODE_MAP) {
        struct foreach_config_data data = {
                .root_name = name
        };

        d->foreach_dict_node(d, value, foreach_config_clb, &data);

    } else if (type != NODE_SEQ) {
        float tmp_f;
        int tmp_int;
        const char *str;

        if (type == NODE_STRING) {
            str = d->as_string(d, value, "");
            _cvar_from_str(name, str);

        } else {
            const uint64_t key = ct_hashlib_a0->id64_from_str(name);

            if (ct_cdb_a0->prop_exist(_G.config_object, key)) {
                enum ct_cdb_type t = ct_cdb_a0->prop_type(
                        _G.config_object, key);

                ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(_G.config_object);

                switch (t) {
                    case CDB_TYPE_NONE:
                        break;

                    case CDB_TYPE_FLOAT:
                        tmp_f = d->as_float(d, value, 0.0f);
                        ct_cdb_a0->set_float(writer, key, tmp_f);
                        break;

                    case CDB_TYPE_UINT64:
                        tmp_int = (int) d->as_float(d, value, 0.0f);
                        ct_cdb_a0->set_uint64(writer, key, tmp_int);
                        break;

                    case CDB_TYPE_STR:
                        str = d->as_string(d, value, "");
                        ct_cdb_a0->set_str(writer, key, str);
                        break;
                    default:
                        break;
                }

                ct_cdb_a0->write_begin(_G.config_object);
            }
        }
    }
}


static int load_from_yaml_file(const char *yaml,
                               struct ct_alloc *alloc) {

    struct ct_vio *f = ct_os_a0->vio_a0->from_file(yaml, VIO_OPEN_READ);
    struct ct_yng_doc *d = ct_yng_a0->from_vio(f, alloc);
    f->close(f);

    struct foreach_config_data config_data = {
            .root_name = NULL
    };


    d->foreach_dict_node(d, d->get(d, 0), foreach_config_clb,
                         &config_data);

    ct_yng_a0->destroy(d);

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


static uint64_t config_object() {
    return _G.config_object;
}

static struct ct_config_a0 config_a0 = {
        .config_object = config_object,
        .parse_args = parse_args,
        .log_all = log_all,
        .load_from_yaml_file = load_from_yaml_file
};

struct ct_config_a0 *ct_config_a0 = &config_a0;


void CETECH_MODULE_INITAPI(config)(struct ct_api_a0 *api) {
}

void CETECH_MODULE_LOAD (config)(struct ct_api_a0 *api,
                                  int reload) {
    CT_UNUSED(reload);
    _G = (struct _G) {0};

    ct_log_a0->debug(LOG_WHERE, "Init");

    _G.db = ct_cdb_a0->db();
    _G.config_object = ct_cdb_a0->create_object(_G.db, 0);
    _G.config_desc = ct_cdb_a0->create_object(_G.db, 1);

    api->register_api("ct_config_a0", &config_a0);

    _G.type = CT_ID32_0("config");
}

void CETECH_MODULE_UNLOAD (config)(struct ct_api_a0 *api,
                                    int reload) {
    CT_UNUSED(api, reload);
    ct_log_a0->debug(LOG_WHERE, "Shutdown");

    ct_cdb_a0->destroy_db(_G.db);
}
