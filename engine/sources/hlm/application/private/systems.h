#ifndef CETECH_SYSTEMS_H
#define CETECH_SYSTEMS_H

#include "hlm/configsystem/configsystem.h"
#include "hlm/consoleserver/consoleserver.h"
#include "hlm/luasystem/luasystem.h"
#include "llm/llm.h"
#include "hlm/input/input.h"
#include "hlm/taskmanager/taskmanager.h"

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
        _REGISTER_SYSTEM(llm),
        _REGISTER_SYSTEM(keyboard),
        _REGISTER_SYSTEM(mouse)
};

#endif //CETECH_SYSTEMS_H
