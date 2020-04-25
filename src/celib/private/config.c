//==============================================================================
// Includes
//==============================================================================
#include <stdio.h>
#include <stdlib.h>

#include <celib/api.h>
#include <celib/memory/allocator.h>
#include <celib/memory/memory.h>
#include <celib/containers/array.h>

#include <celib/log.h>
#include <celib/id.h>
#include <celib/config.h>
#include <celib/module.h>
#include <celib/cdb.h>
#include <celib/macros.h>
#include <celib/cdb_yaml.h>
#include <celib/os/vio.h>
#include <celib/containers/hash.h>




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
    ce_hash_t config;
} _G;


//==============================================================================
// Privates
//==============================================================================


//==============================================================================
// Interface
//==============================================================================

static void log_all() {
}

static uint64_t _uid_from_str(const char *str) {
    uint64_t v = strtoul(str, NULL, 0);
    return v;
}

static bool _is_ref(const char *str) {
    return (str[0] == '0') && (str[1] == 'x');
}


float read_float(uint64_t name,
                 float defaultt) {
    return ce_hash_lookup(&_G.config, name, defaultt);
}

uint64_t read_uint(uint64_t name,
                   uint64_t defaultt) {
    return ce_hash_lookup(&_G.config, name, defaultt);
}

const char *read_str(uint64_t name,
                     const char *defaultt) {
    return (const char *) ce_hash_lookup(&_G.config, name, (uint64_t) defaultt);
}

void set_float(uint64_t name,
               float value) {
    ce_hash_add(&_G.config, name, value, ce_memory_a0->system);
}

void set_uint(uint64_t name,
              uint64_t value) {
    ce_hash_add(&_G.config, name, value, ce_memory_a0->system);
}

void set_str(uint64_t name,
             const char *value) {
    const char *new_str = ce_memory_a0->str_dup(value, ce_memory_a0->system);
    ce_hash_add(&_G.config, name, (uint64_t) new_str, ce_memory_a0->system);
}

static void _cvar_from_str(const char *name,
                           const char *value) {
    int d = 0;
    float f = 0;

    const uint64_t key = ce_id_a0->id64(name);

    if (value == NULL) {
        set_uint(key, 1);
        return;
    }

    if (_is_ref(value)) {
        uint64_t ref = _uid_from_str(value);
        set_uint(key, ref);
        return;
    } else if (sscanf(value, "%d", &d)) {
        set_uint(key, d);
        return;

    } else if (sscanf(value, "%f", &f)) {
        set_float(key, d);
        return;
    }

    set_str(key, value);
}

static int load_from_yaml_file(ce_cdb_t0 db,
                               const char *path,
                               struct ce_alloc_t0 *alloc) {
    ce_vio_t0 *f = ce_os_vio_a0->from_file(path, VIO_OPEN_READ);

    ct_cdb_node_t *nodes = NULL;
    ce_cdb_yaml_a0->load_to_nodes(path, f, &nodes, alloc);
    uint64_t obj = ce_cdb_a0->load_from_cnodes(nodes, db);
    ce_os_vio_a0->close(f);
    ce_array_free(nodes, alloc);

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, obj);

    uint64_t variables_n = ce_cdb_a0->read_objset_num(reader, CE_CONFIG_VARIABLES);
    uint64_t variables_k[variables_n];
    ce_cdb_a0->read_objset(reader, CE_CONFIG_VARIABLES, variables_k);
    for (int j = 0; j < variables_n; ++j) {
        uint64_t t = ce_cdb_a0->obj_type(db, variables_k[j]);

        const ce_cdb_obj_o0 *var_r = ce_cdb_a0->read(db, variables_k[j]);

        const char *name = ce_cdb_a0->read_str(var_r, CE_CONFIG_NAME, "");
        uint64_t name_h = ce_id_a0->id64(name);


        switch (t) {
            case CE_CONFIG_VARIABLE_STR: {
                const char *value = ce_cdb_a0->read_str(var_r, CE_CONFIG_VALUE, "");
                set_str(name_h, value);
                break;
            }

            case CE_CONFIG_VARIABLE_INT: {
                uint64_t value = ce_cdb_a0->read_uint64(var_r, CE_CONFIG_VALUE, 0);
                set_uint(name_h, value);
                break;
            }

            case CE_CONFIG_VARIABLE_REF: {
                uint64_t value = ce_cdb_a0->read_ref(var_r, CE_CONFIG_VALUE, 0);
                set_uint(name_h, value);
            }

            default:
                continue;
        }

    }

    ce_cdb_a0->destroy_object(db, obj);

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

bool exist(uint64_t name) {
    return ce_hash_contain(&_G.config, name);
}

static struct ce_config_a0 config_a0 = {
//        .obj = config_object,
        .parse_args = parse_args,
        .log_all = log_all,
        .from_file = load_from_yaml_file,

        .read_float = read_float,
        .read_uint = read_uint,
        .read_str = read_str,

        .set_float = set_float,
        .set_uint = set_uint,
        .set_str = set_str,
        .exist = exist,
};

struct ce_config_a0 *ce_config_a0 = &config_a0;


static const ce_cdb_prop_def_t0 config_cdb_type_def[] = {
        {
                .name = "variables",
                .type = CE_CDB_TYPE_SET_SUBOBJECT,
        },
};

static const ce_cdb_prop_def_t0 config_variable_ref_cdb_type_def[] = {
        {
                .name = "name",
                .type = CE_CDB_TYPE_STR,
        },
        {
                .name = "value",
                .type = CE_CDB_TYPE_REF,
        },
};

static const ce_cdb_prop_def_t0 config_variable_str_cdb_type_def[] = {
        {
                .name = "name",
                .type = CE_CDB_TYPE_STR,
        },
        {
                .name = "value",
                .type = CE_CDB_TYPE_STR,
        },
};

static const ce_cdb_prop_def_t0 config_variable_int_cdb_type_def[] = {
        {
                .name = "name",
                .type = CE_CDB_TYPE_STR,
        },
        {
                .name = "value",
                .type = CE_CDB_TYPE_UINT64,
        },
};

void CE_MODULE_LOAD (config)(struct ce_api_a0 *api,
                             int reload) {
    CE_UNUSED(reload);
    _G = (struct _G) {};

    ce_log_a0->debug(LOG_WHERE, "Init");

    api->add_api("ce_config_a0", &config_a0, sizeof(config_a0));

    ce_cdb_a0->reg_obj_type(CE_CONFIG_TYPE, CE_ARR_ARG(config_cdb_type_def));
    ce_cdb_a0->reg_obj_type(CE_CONFIG_VARIABLE_REF, CE_ARR_ARG(config_variable_ref_cdb_type_def));
    ce_cdb_a0->reg_obj_type(CE_CONFIG_VARIABLE_STR, CE_ARR_ARG(config_variable_str_cdb_type_def));
    ce_cdb_a0->reg_obj_type(CE_CONFIG_VARIABLE_INT, CE_ARR_ARG(config_variable_int_cdb_type_def));
}

void CE_MODULE_UNLOAD (config)(struct ce_api_a0 *api,
                               int reload) {
    CE_UNUSED(api, reload);
    ce_log_a0->debug(LOG_WHERE, "Shutdown");
}
