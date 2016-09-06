#ifndef CETECH_SYSTEMS_H
#define CETECH_SYSTEMS_H

#include "engine/machine/machine.h"

#include "engine/config_system/config_system.h"
#include "engine/console_server/console_server.h"
#include "engine/lua_system/lua_system.h"
#include "engine/input/input.h"
#include "engine/task_manager/task_manager.h"
#include "engine/resource_compiler/resource_compiler.h"
#include "engine/machine/machine.h"
#include "engine/resource_manager/resource_manager.h"
#include "engine/filesystem/filesystem.h"
#include "engine/renderer/renderer.h"

#define _SYSTEMS_SIZE sizeof(_SYSTEMS)/sizeof(_SYSTEMS[0])
#define _REGISTER_SYSTEM(n) {.name= #n, .init=n##_init, .shutdown=n##_shutdown}

static const struct {
    const char *name;

    int (*init)();

    void (*shutdown)();
} _SYSTEMS[] = {
        _REGISTER_SYSTEM(config),
        _REGISTER_SYSTEM(machine),
        _REGISTER_SYSTEM(taskmanager),
        _REGISTER_SYSTEM(filesystem),
        _REGISTER_SYSTEM(resource_compiler),
        _REGISTER_SYSTEM(resource),
        _REGISTER_SYSTEM(consolesrv),
        _REGISTER_SYSTEM(keyboard),
        _REGISTER_SYSTEM(mouse),
        _REGISTER_SYSTEM(luasys),
        _REGISTER_SYSTEM(renderer)
};

#endif //CETECH_SYSTEMS_H
