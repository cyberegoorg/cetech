//==============================================================================
// Includes
//==============================================================================
#include <stdio.h>
#include <string.h>

#include <celib/api_system.h>
#include <celib/memory.h>
#include <celib/os.h>

#include <celib/log.h>
#include <celib/hashlib.h>
#include <celib/config.h>
#include <celib/module.h>
#include <celib/yng.h>
#include <celib/cdb.h>
#include <celib/macros.h>


//==============================================================================
// Defines
//==============================================================================


#define LOG_WHERE "cvar"

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
    struct ce_cdb_t db;
    uint64_t config_object;
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

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(_G.config_object);

    const uint64_t key = ce_id_a0->id64(name);

    if (value == NULL) {
        ce_cdb_a0->set_uint64(writer, key, 1);
        goto end;
    }


    if (sscanf(value, "%d", &d)) {
        ce_cdb_a0->set_uint64(writer, key, d);
        goto end;

    } else if (sscanf(value, "%f", &f)) {
        ce_cdb_a0->set_float(writer, key, d);
        goto end;
    }

    ce_cdb_a0->set_str(writer, key, value);

    end:
    ce_cdb_a0->write_commit(writer);
}

struct foreach_config_data {
    char *root_name;
};


static void foreach_config_clb(struct ce_yng_node key,
                               struct ce_yng_node value,
                               void *_data) {

    struct foreach_config_data *output = (struct foreach_config_data *) _data;
    struct ce_yng_doc *d = key.d;

    const char *key_str = d->as_string(d, key, "");

    char name[1024] = {};
    if (output->root_name != NULL) {
        snprintf(name, CE_ARRAY_LEN(name),
                 "%s.%s", output->root_name, key_str);
    } else {
        snprintf(name, CE_ARRAY_LEN(name), "%s", key_str);
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
            const uint64_t key = ce_id_a0->id64(name);

            if (ce_cdb_a0->prop_exist(_G.config_object, key)) {
                enum ce_cdb_type t = ce_cdb_a0->prop_type(
                        _G.config_object, key);

                ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(_G.config_object);

                switch (t) {
                    case CDB_TYPE_NONE:
                        break;

                    case CDB_TYPE_FLOAT:
                        tmp_f = d->as_float(d, value, 0.0f);
                        ce_cdb_a0->set_float(writer, key, tmp_f);
                        break;

                    case CDB_TYPE_UINT64:
                        tmp_int = (int) d->as_float(d, value, 0.0f);
                        ce_cdb_a0->set_uint64(writer, key, tmp_int);
                        break;

                    case CDB_TYPE_STR:
                        str = d->as_string(d, value, "");
                        ce_cdb_a0->set_str(writer, key, str);
                        break;
                    default:
                        break;
                }

                ce_cdb_a0->write_begin(_G.config_object);
            }
        }
    }
}


static int load_from_yaml_file(const char *path,
                               struct ce_alloc *alloc) {

    struct ce_vio *f = ce_os_a0->vio->from_file(path, VIO_OPEN_READ);
    struct ce_yng_doc *d = ce_yng_a0->from_vio(f, alloc);
    f->close(f);

    struct foreach_config_data config_data = {
            .root_name = NULL
    };


    d->foreach_dict_node(d, d->get(d, 0), foreach_config_clb,
                         &config_data);

    ce_yng_a0->destroy(d);

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

static struct ce_config_a0 config_a0 = {
        .obj = config_object,
        .parse_args = parse_args,
        .log_all = log_all,
        .load_from_yaml_file = load_from_yaml_file
};

struct ce_config_a0 *ce_config_a0 = &config_a0;


void CE_MODULE_INITAPI(config)(struct ce_api_a0 *api) {
}

void CE_MODULE_LOAD (config)(struct ce_api_a0 *api,
                                  int reload) {
    CE_UNUSED(reload);
    _G = (struct _G) {{0}};

    ce_log_a0->debug(LOG_WHERE, "Init");

    _G.db = ce_cdb_a0->db();
    _G.config_object = ce_cdb_a0->create_object(_G.db, 0);

    api->register_api("ct_config_a0", &config_a0);
}

void CE_MODULE_UNLOAD (config)(struct ce_api_a0 *api,
                                    int reload) {
    CE_UNUSED(api, reload);
    ce_log_a0->debug(LOG_WHERE, "Shutdown");

    ce_cdb_a0->destroy_db(_G.db);
}
