//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <include/mpack/mpack.h>

#include "include/nanomsg/nn.h"
#include "include/nanomsg/reqrep.h"
#include "include/nanomsg/pubsub.h"
#include "include/nanomsg/pipeline.h"

#include "engine/develop/console_server.h"
#include "celib/errors/errors.h"
#include "celib/string/string.h"

#include "engine/core/cvar.h"

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "develop_system"

//==============================================================================
// Globals
//==============================================================================

#define _G DevelopSystemGlobals

static struct G {
    int pub_socket;

    cvar_t cv_pub_port;
    cvar_t cv_pub_addr;
} _G = {0};

//==============================================================================
// Interface
//==============================================================================


int developsys_init(int stage) {
    if (stage == 0) {
        _G = (struct G) {0};

        _G.cv_pub_port = cvar_new_int("develop.pub.port", "Console server rpc port", 4447);
        _G.cv_pub_addr = cvar_new_str("develop.pub.addr", "Console server rpc addr", "ws://*");

        return 1;
    }

    char addr[128] = {0};

    log_debug(LOG_WHERE, "Init");

    int socket = nn_socket(AF_SP, NN_PUB);
    if (socket < 0) {
        log_error(LOG_WHERE, "Could not create nanomsg socket: %s", nn_strerror(errno));
        return 0;
    }
    snprintf(addr, 128, "%s:%d", cvar_get_string(_G.cv_pub_addr), cvar_get_int(_G.cv_pub_port));

    log_debug(LOG_WHERE, "PUB address: %s", addr);

    if (nn_bind(socket, addr) < 0) {
        log_error(LOG_WHERE, "Could not bind socket to '%s': %s", addr, nn_strerror(errno));
        return 0;
    }

    _G.pub_socket = socket;

    return 1;
}

void developsys_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    nn_close(_G.pub_socket);
}

void developsys_update() {
}
