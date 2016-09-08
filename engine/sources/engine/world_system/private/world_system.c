#include "celib/containers/queue.h"

#define _G WorldGlobals
static struct G {
    char _;
} _G = {0};

int world_init() {
    _G = (struct G) {0};


    return 0;
}

void world_shutdown() {
    _G = (struct G) {0};
}
