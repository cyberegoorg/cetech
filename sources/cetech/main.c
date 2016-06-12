#include <stdlib.h>
#include <stdio.h>

#include "../celib/memory/allocator.h"
#include "../celib/memory/mallocator.h"
#include "../celib/log/log.h"
#include "../celib/log/handlers.h"
#include "../celib/containers/array.h"

#include "include/nanomsg/nn.h"
#include "include/nanomsg/pipeline.h"

#include "include/mpack/mpack.h"
#include "include/mpack/mpack-writer.h"
#include "../celib/math/types.h"
#include "../celib/math/vec2f.h"
#include "../celib/math/vec4f.h"

////////////////////////////////////////////////////////////////////////////////////////
char get_worker_id() {
    return 0;
}



int main(int argc, char **argv) {
    log_init(get_worker_id);
    log_register_handler(log_stdout_handler, NULL);

//    int socket = nn_socket(AF_SP, NN_PUSH);
//    CE_ASSERT("console_server", socket >= 0);
//    CE_ASSERT("console_server", nn_bind(socket, "tcp://*:5556") >= 0);
//    log_register_handler(nano_log_handler, (void *) socket);

    ALLOCATOR_CREATE(a, mallocator);

    //CE_ASSERT("main", 0);

    log_info("main", "sadasdsad");
    log_debug("main", "sadasdsad");
    log_warning("main", "sadasdsad");
    log_error("main", "sadasdsad");

    void *ptr = alloc_alloc(a, 100);
    alloc_free(a, ptr);

    ARRAY_T(int) array;
    ARRAY_INIT(int, &array, a);

    //ARRAY_POP_BACK(int, &array);

    for (int i = 0; i < 100; ++i) {
        ARRAY_PUSH_BACK(int, &array, i);
    }

    ARRAY_AT(&array, 0) = 10;

    for (int i = 0; i < ARRAY_SIZE(&array); ++i) {
        log_info("main", "I: %d", ARRAY_AT(&array, i));
    }

    log_info("main", "capacity %zu", ARRAY_CAPACITY(&array));
    log_info("main", "size %zu", ARRAY_SIZE(&array));

    ARRAY_DESTROY(int, &array);

    mallocator_destroy(a);

    vec4f_t v1 = {0};
    vec4f_t v2 = {.x = 1.0f, .y = 2.0f, .z = 3.0f, .w = 4.0f};

    v1 = vec4f_add(v1, v2);
    v1 = vec4f_sub(v1, v2);

    log_info("main", "%zu, %f, %f", sizeof(v1), v1.x, v1.y);


    log_shutdown();
    return 0;
}