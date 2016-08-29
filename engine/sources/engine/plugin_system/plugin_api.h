#ifndef CETECH_PLUGIN_API_H
#define CETECH_PLUGIN_API_H

#include <stdio.h>

#define PLUGIN_API_ID         1
//#define LOG_API_ID            2
//#define MEMORY_API_ID         3
//#define LUA_API_ID            4
//#define CONFIG_API_ID         5
//#define CONSOLE_SERVER_API_ID 6

typedef void *(*get_api_fce_t)(int api, int version);

struct plugin_api_v0 {
    void (*init            )(get_api_fce_t);

    void (*shutdown        )(void);

    void *(*reload_begin    )(get_api_fce_t);

    void (*reload_end      )(get_api_fce_t, void *);

    void (*update          )();
};

//#include "../log_system/log_api.h"
//#include "../memory_system/mem_api.h"
//#include "../lua_system/lua_api.h"
//#include "../config_system/api.h"

#endif //CETECH_PLUGIN_API_H
