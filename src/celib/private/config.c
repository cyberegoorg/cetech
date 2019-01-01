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
#include <celib/cdb.h>
#include <celib/macros.h>
#include <celib/ydb.h>


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

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                  _G.config_object);

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


static void foreach_config_clb(uint64_t key,
                               uint64_t value,
                               char *root_name) {


    const char *key_str = ce_id_a0->str_from_id64(key);

    char name[1024] = {};
    if (root_name != NULL) {
        snprintf(name, CE_ARRAY_LEN(name),
                 "%s.%s", root_name, key_str);
    } else {
        snprintf(name, CE_ARRAY_LEN(name), "%s", key_str);
    }


    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), value);
    enum ce_cdb_type type = ce_cdb_a0->prop_type(reader, key);

    if (type == CDB_TYPE_SUBOBJECT) {
        uint64_t sub_obj = ce_cdb_a0->read_subobject(reader, key, 0);

        const ce_cdb_obj_o *subr = ce_cdb_a0->read(ce_cdb_a0->db(), sub_obj);

        const uint64_t n = ce_cdb_a0->prop_count(subr);
        const uint64_t *keys = ce_cdb_a0->prop_keys(subr);

        for (int i = 0; i < n; ++i) {
            foreach_config_clb(keys[i], sub_obj, name);
        }

    } else {
        float tmp_f;
        int tmp_int;
        const char *str;

        if (type == CDB_TYPE_STR) {
            str = ce_cdb_a0->read_str(reader, key, "");
            _cvar_from_str(name, str);

        } else {
            const uint64_t key = ce_id_a0->id64(name);

            const ce_cdb_obj_o *conf_r = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                         _G.config_object);

            if (ce_cdb_a0->prop_exist(conf_r, key)) {
                enum ce_cdb_type t = ce_cdb_a0->prop_type(conf_r, key);

                ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                              _G.config_object);

                switch (t) {
                    case CDB_TYPE_NONE:
                        break;

                    case CDB_TYPE_FLOAT:
                        tmp_f = ce_cdb_a0->read_float(reader, key, 0.0f);
                        ce_cdb_a0->set_float(writer, key, tmp_f);
                        break;

                    case CDB_TYPE_UINT64:
                        tmp_int = (int) ce_cdb_a0->read_float(reader, key,
                                                              0.0f);
                        ce_cdb_a0->set_uint64(writer, key, tmp_int);
                        break;

                    case CDB_TYPE_STR:
                        str = ce_cdb_a0->read_str(reader, key, "");
                        ce_cdb_a0->set_str(writer, key, str);
                        break;
                    default:
                        break;
                }

                ce_cdb_a0->write_commit(writer);
            }
        }
    }
}


static int load_from_yaml_file(const char *path,
                               struct ce_alloc *alloc) {

    struct ce_vio *f = ce_os_a0->vio->from_file(path, VIO_OPEN_READ);
    uint64_t obj = ce_ydb_a0->cdb_from_vio(f, alloc);
    f->close(f);

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    const uint64_t n = ce_cdb_a0->prop_count(reader);
    const uint64_t *keys = ce_cdb_a0->prop_keys(reader);

    for (int i = 0; i < n; ++i) {
        foreach_config_clb(keys[i], obj, NULL);
    }

    ce_cdb_a0->destroy_object(ce_cdb_a0->db(), obj);

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
    _G = (struct _G) {};

    ce_log_a0->debug(LOG_WHERE, "Init");

    _G.db = ce_cdb_a0->db();
    _G.config_object = ce_cdb_a0->create_object(_G.db, 0);

    api->register_api(CE_CONFIG_API, &config_a0);
}

void CE_MODULE_UNLOAD (config)(struct ce_api_a0 *api,
                               int reload) {
    CE_UNUSED(api, reload);
    ce_log_a0->debug(LOG_WHERE, "Shutdown");

    ce_cdb_a0->destroy_db(_G.db);
}
