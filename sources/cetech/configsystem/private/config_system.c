#include <celib/memory/memory.h>
#include "celib/string/string.h"

#include "celib/log/log.h"

#include "cetech/configsystem/configsystem.h"

#define MAX_VARIABLES 1024
#define MAX_NAME_LEN 128
#define MAX_DESC_LEN 256
#define LOG_WHERE "config_system"

enum cvar_type {
    CV_NONE = 0,
    CV_FLOAT,
    CV_INT,
    CV_STRING
};

static struct {
    char name[MAX_VARIABLES][MAX_NAME_LEN];
    char desc[MAX_VARIABLES][MAX_DESC_LEN];

    enum cvar_type types[MAX_VARIABLES];

    union {
        float f;
        int i;
        char *s;
    } values[MAX_VARIABLES];
} _G = {0};

void _dealloc_all_string() {
    for (int i = 0; i < MAX_VARIABLES; ++i) {
        if (_G.types[i] != CV_STRING) {
            continue;
        }

        CE_DEALLOCATE(memsys_main_allocator(), _G.values[i].s);
    }
}

void config_init() {
    log_debug(LOG_WHERE, "Init");
}

void config_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    _dealloc_all_string();
}

///

config_var_t _find_first_free() {
    for (config_var_t i = 1; i < MAX_VARIABLES; ++i) {
        if (_G.name[i][0] != '\0') {
            continue;
        }

        return i;
    }

    log_error(LOG_WHERE, "Could not create new config variable");
    return 0;
}

config_var_t config_find_or_create(const char *name, int *new) {
    *new = 0;

    for (config_var_t i = 1; i < MAX_VARIABLES; ++i) {
        if (_G.name[i][0] != '\0') {
            continue;
        }

        if (str_compare(_G.name[i], name) != 0) {
            continue;
        }

        return i;
    }

    const config_var_t var = _find_first_free();

    if (var != 0) {
        *new = 1;
        return var;
    }

    return 0;
}

config_var_t config_new_float(const char *name, const char *desc, float f) {
    int new;
    config_var_t find = config_find_or_create(name, &new);

    if (find == 0) {
        return find;
    }

    if (new) {
        str_set(_G.name[find], name);
        _G.types[find] = CV_FLOAT;
        _G.values[find].f = f;
    }

    str_set(_G.desc[find], desc);

    return find;
}

config_var_t config_new_int(const char *name, const char *desc, int i) {
    int new;
    config_var_t find = config_find_or_create(name, &new);

    if (find == 0) {
        return find;
    }

    if (new) {
        str_set(_G.name[find], name);
        _G.types[find] = CV_INT;
        _G.values[find].i = i;
    }

    str_set(_G.desc[find], desc);

    return find;
}

config_var_t config_new_string(const char *name, const char *desc, const char *s) {
    int new;
    config_var_t find = config_find_or_create(name, &new);

    if (find == 0) {
        return find;
    }

    if (new) {
        str_set(_G.name[find], name);
        _G.types[find] = CV_STRING;
        _G.values[find].s = str_duplicate(s, memsys_main_allocator());
    }

    str_set(_G.desc[find], desc);

    return find;
}

float config_get_float(config_var_t var) {
    return _G.values[var].f;
}

int config_get_int(config_var_t var) {
    return _G.values[var].i;
}

const char *config_get_string(config_var_t var) {
    return _G.values[var].s;
}

void config_set_float(config_var_t var, float f) {
    _G.values[var].f = f;
}

void config_set_int(config_var_t var, int i) {
    _G.values[var].i = i;
}

void config_set_string(config_var_t var, const char *s) {
    char *_s = _G.values[var].s;

    if (_s != NULL) {
        allocator_deallocate(memsys_main_allocator(), _s);
    }

    _G.values[var].s = str_duplicate(s, memsys_main_allocator());
}

