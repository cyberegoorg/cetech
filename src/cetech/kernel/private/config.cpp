//==============================================================================
// Includes
//==============================================================================
#include <stdio.h>
#include <string.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/path.h>
#include <cetech/kernel/vio.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/kernel/config.h>
#include <celib/memory.h>
#include <cetech/kernel/module.h>
#include <celib/buffer.inl>
#include <cetech/kernel/yamlng.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_yng_a0);

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
    char name[MAX_VARIABLES][MAX_NAME_LEN];
    char desc[MAX_VARIABLES][MAX_DESC_LEN];

    cvar_type types[MAX_VARIABLES];

    union {
        float f;
        int i;
        char *s;
    } values[MAX_VARIABLES];
    uint64_t type;
} _G;


//==============================================================================
// Privates
//==============================================================================


void _deallocate_all_string() {
    for (int i = 0; i < MAX_VARIABLES; ++i) {
        if (_G.types[i] != CV_STRING) {
            continue;
        }

        CEL_FREE(ct_memory_a0.main_allocator(), _G.values[i].s);
    }
}

ct_cvar _find_first_free() {

    for (uint64_t i = 1; i < MAX_VARIABLES; ++i) {
        if (_G.name[i][0] != '\0') {
            continue;
        }

        return make_cvar(i);
    }

    ct_log_a0.error(LOG_WHERE, "Could not create _new config variable");

    return make_cvar(0);
}


//==============================================================================
// Interface
//==============================================================================
namespace config {
    ct_cvar find(const char *name) {
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

    ct_cvar find_or_create(const char *name,
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

        const ct_cvar var = _find_first_free();

        if (var.idx != 0) {
            str_set(_G.name[var.idx], name);

            if (_new) *_new = 1;
            return var;
        }

        return make_cvar(0);
    }

    ct_cvar new_float(const char *name,
                      const char *desc,
                      float f) {
        int _new;
        ct_cvar find = find_or_create(name, &_new);

        if (_new) {
            str_set(_G.name[find.idx], name);
            _G.types[find.idx] = CV_FLOAT;
            _G.values[find.idx].f = f;
        }

        str_set(_G.desc[find.idx], desc);

        return find;
    }

    ct_cvar new_int(const char *name,
                    const char *desc,
                    int i) {
        int _new;
        ct_cvar find = find_or_create(name, &_new);

        if (_new) {
            str_set(_G.name[find.idx], name);
            _G.types[find.idx] = CV_INT;
            _G.values[find.idx].i = i;
        }

        str_set(_G.desc[find.idx], desc);

        return find;
    }

    ct_cvar new_str(const char *name,
                    const char *desc,
                    const char *s) {
        int _new;
        ct_cvar find = find_or_create(name, &_new);

        if (_new) {
            str_set(_G.name[find.idx], name);
            _G.types[find.idx] = CV_STRING;
            _G.values[find.idx].s = ct_memory_a0.str_dup(s,
                                                         ct_memory_a0.main_allocator());
        }

        str_set(_G.desc[find.idx], desc);

        return find;
    }

    float get_float(ct_cvar var) {
        return _G.values[var.idx].f;
    }

    int get_int(ct_cvar var) {
        return _G.values[var.idx].i;
    }

    const char *get_string(ct_cvar var) {
        return _G.values[var.idx].s;
    }

    enum cvar_type get_type(ct_cvar var) {
        return _G.types[var.idx];
    }

    void set_float(ct_cvar var,
                   float f) {
        _G.values[var.idx].f = f;
    }

    void set_int(ct_cvar var,
                 int i) {
        _G.values[var.idx].i = i;
    }

    void set_string(ct_cvar var,
                    const char *s) {
        char *_s = _G.values[var.idx].s;

        if (_s != NULL) {
            CEL_FREE(ct_memory_a0.main_allocator(), _s);
        }

        _G.values[var.idx].s = ct_memory_a0.str_dup(s,
                                                    ct_memory_a0.main_allocator());
    }

    void log_all() {
        celib::Buffer out(ct_memory_a0.main_allocator());

        celib::buffer::printf(out, "config:\n");

        for (uint64_t i = 1; i < MAX_VARIABLES; ++i) {
            if (_G.name[i][0] == '\0') {
                continue;
            }

            const char *name = _G.name[i];

            switch (_G.types[i]) {
                case CV_FLOAT:
                    celib::buffer::printf(out, "    - %s = %f\n", name,
                                          _G.values[i].f);
                    break;
                case CV_INT:
                    celib::buffer::printf(out, "    - %s = %d\n", name,
                                          _G.values[i].i);
                    break;
                case CV_STRING:
                    celib::buffer::printf(out, "    - %s = %s\n", name,
                                          _G.values[i].s);
                    break;
                default:
                    break;
            }
        }

        ct_log_a0.info(LOG_WHERE, "%s", celib::buffer::c_str(out));
    }

    void _cvar_from_str(const char *name,
                        const char *value) {
        union {
            float f;
            int i;
            const char *s;
        } tmp_var;

        ct_cvar cvar = find(name);
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
                    ct_log_a0.error(LOG_WHERE, "Invalid type for cvar \"%s\"",
                                    name);
                    break;
            }

        } else {
            if (value == NULL) {
                new_int(name, "", 1);
                return;
            }

            int d = 0;
            float f = 0;
            if (sscanf(value, "%d", &d)) {
                new_int(name, "", d);
                return;

            } else if (sscanf(value, "%f", &f)) {
                new_float(name, "", f);
                return;
            }

            new_str(name, "", value);
            //ct_log_a0.error(LOG_WHERE, "Invalid cvar \"%s\"", name);
        }
    }

    struct foreach_config_data {
        char *root_name;
    };


    void foreach_config_clb(struct ct_yamlng_node key,
                            struct ct_yamlng_node value,
                            void *_data) {

        struct foreach_config_data *output = (foreach_config_data *) _data;
        ct_yng_doc* d = key.d;

        const char* key_str = d->as_string(d->inst, key, "");

        char name[1024] = {};
        if (output->root_name != NULL) {
            snprintf(name, CETECH_ARRAY_LEN(name), "%s.%s", output->root_name, key_str);
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
            const char * str;

            if(type == NODE_STRING) {
                str  = d->as_string(d->inst, value, "");
                _cvar_from_str(name, str);

            } else {
                ct_cvar cvar = find(name);
                if (cvar.idx != 0) {
                    enum cvar_type t = get_type(cvar);
                    switch (t) {
                        case CV_NONE:
                            break;
                        case CV_FLOAT:
                            tmp_f = d->as_float(d->inst, value, 0.0f);
                            set_float(cvar, tmp_f);
                            break;
                        case CV_INT:
                            tmp_int = (int) d->as_float(d->inst, value, 0.0f);
                            set_int(cvar, tmp_int);
                            break;
                        case CV_STRING:
                            str = d->as_string(d->inst, value, "");
                            set_string(cvar, str);
                            break;
                    }
                }
            }
        }
    }


    int load_from_yaml_file(const char *yaml,
                            cel_alloc *alloc) {

        ct_vio *f = ct_vio_a0.from_file(yaml, VIO_OPEN_READ);
        ct_yng_doc *d = ct_yng_a0.from_vio(f,alloc);
        f->close(f->inst);

        struct foreach_config_data config_data = {
                .root_name = NULL
        };


        d->foreach_dict_node(d->inst, d->get(d->inst, 0), foreach_config_clb,
                             &config_data);

        ct_yng_a0.destroy(d);

        return 1;
    }



    int parse_args(int argc,
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


    static ct_config_a0 config_a0 = {
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
            .load_from_yaml_file = load_from_yaml_file
    };

    int init(ct_api_a0 *api) {
        CEL_UNUSED(api);
        return 1;
    }

    void shutdown() {

    }
};


CETECH_MODULE_DEF(
        config,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_yng_a0);
        },
        {
            CEL_UNUSED(reload);

            _G = {};

            ct_log_a0.debug(LOG_WHERE, "Init");

            api->register_api("ct_config_a0", &config::config_a0);

            _G.type = ct_hash_a0.id64_from_str("config");
        },
        {
            CEL_UNUSED(api, reload);

            ct_log_a0.debug(LOG_WHERE, "Shutdown");

            _deallocate_all_string();
        }
)
