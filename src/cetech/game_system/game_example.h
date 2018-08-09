#ifndef CETECH_GAME_H
#define CETECH_GAME_H

#include <stdint.h>

#include <corelib/module.inl>

struct ct_game_i0 {
    uint64_t (*name)();

    void (*init)();
    void (*shutdown)();

    void (*update)();
    void (*render)();
};

struct ct_game_system_a0 {
    void (*init)();
    void (*shutdown)();
    void (*update)();
    void (*render)();

    void (*pause)();
    void (*play)();
};

CT_MODULE(ct_game_system_a0);

#endif //CETECH_GAME_H
