#ifndef CETECH_SYSTEMS_H
#define CETECH_SYSTEMS_H

#include "llm/llm.h"

#include "engine/configsystem/configsystem.h"
#include "engine/consoleserver/consoleserver.h"
#include "engine/luasystem/luasystem.h"
#include "engine/input/input.h"
#include "engine/taskmanager/taskmanager.h"

#define _SYSTEMS_SIZE sizeof(_SYSTEMS)/sizeof(_SYSTEMS[0])
#define _REGISTER_SYSTEM(n) {.name= #n, .init=n##_init, .shutdown=n##_shutdown}

static const struct {
    const char *name;

    int (*init)();

    void (*shutdown)();
} _SYSTEMS[] = {
        _REGISTER_SYSTEM(config),
        _REGISTER_SYSTEM(taskmanager),
        _REGISTER_SYSTEM(consolesrv),
        _REGISTER_SYSTEM(luasys),
        _REGISTER_SYSTEM(keyboard),
        _REGISTER_SYSTEM(mouse)
};

#endif //CETECH_SYSTEMS_H
