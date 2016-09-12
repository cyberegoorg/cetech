#ifndef CETECH_SYSTEMS_H
#define CETECH_SYSTEMS_H

#include "engine/machine/machine.h"

#include "engine/cvar/cvar.h"
#include "engine/console_server/console_server.h"
#include "engine/lua_system/lua_system.h"
#include "engine/input/input.h"
#include "engine/task_manager/task_manager.h"
#include "engine/resource_compiler/resource_compiler.h"
#include "engine/machine/machine.h"
#include "engine/resource_manager/resource_manager.h"
#include "engine/filesystem/filesystem.h"
#include "engine/renderer/renderer.h"
#include "engine/entcom/entcom.h"
#include "engine/world_system/world_system.h"
#include "engine/world_system/transform.h"
#include "engine/world_system/unit_system.h"


#define _SYSTEMS_SIZE sizeof(_SYSTEMS)/sizeof(_SYSTEMS[0])
#define _REGISTER_SYSTEM(n) {.name= #n, .init=n##_init, .shutdown=n##_shutdown}

static const struct {
    const char *name;

    int (*init)();

    void (*shutdown)();
} _SYSTEMS[] = {
        _REGISTER_SYSTEM(cvar),
        _REGISTER_SYSTEM(machine),
        _REGISTER_SYSTEM(taskmanager),
        _REGISTER_SYSTEM(filesystem),
        _REGISTER_SYSTEM(resource_compiler),
        _REGISTER_SYSTEM(resource),
        _REGISTER_SYSTEM(consolesrv),
        _REGISTER_SYSTEM(keyboard),
        _REGISTER_SYSTEM(mouse),
        _REGISTER_SYSTEM(luasys),
        _REGISTER_SYSTEM(renderer),
        _REGISTER_SYSTEM(world),
        _REGISTER_SYSTEM(unit),
        _REGISTER_SYSTEM(entcom),
        _REGISTER_SYSTEM(transform),
};

#endif //CETECH_SYSTEMS_H
