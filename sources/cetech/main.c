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
#include "../celib/math/quatf.h"
#include "../celib/math/fmath.h"


////////////////////////////////////////////////////////////////////////////////////////
char get_worker_id() {
    return 0;
}

#include "../celib/memory/ialocator.h"

#define STATOCATOR_T(size) \
    struct statocator##size {\
        struct iallocator i;\
        size_t offset;\
        char buffer[size];\
    };\
\
    void *_statocator_malloc_##size(Alloc_t allocator, size_t _size) {\
        CE_ASSERT("main", allocator != NULL);\
        CE_ASSERT("main", _size != 0);\
    \
        struct statocator##size *a = (struct statocator##size*)allocator;\
    \
        if( a->offset + _size > size) {\
            return NULL;\
        }\
    \
        void *p = a->buffer + a->offset;\
    \
        a->offset += _size;\
    \
        return p;\
    }\
\
    void _statocator_clear_##size(Alloc_t allocator) {\
        CE_ASSERT("main", allocator != NULL);\
    \
        struct statocator##size *a = (struct statocator##size*)allocator;\
        a->offset = 0;\
    }\


#define STATOCATOR(name, size) \
    struct statocator##size name = { .i = {.type_name = "statocator", .alloc = _statocator_malloc_##size, .free = _statocator_free}, .buffer = {0}, .offset = 0} \

#define STATOCATOR_CLEAR(allocator, size) \
    _statocator_clear_##size (allocator)\


#define STATOCATOR_INIT(size) \
    { .i = {.type_name = "statocator", .alloc = _statocator_malloc_##size, .free = _statocator_free}, .buffer = {0}, .offset = 0} \


STATOCATOR_T(_1KiB)

void _statocator_free(Alloc_t allocator, void *ptr) {
}

#include "../celib/yaml/yaml.h"

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

    #define BLOCK (_8KiB*4)

    float v1[BLOCK] = {[0 ... (BLOCK-1)] = 1.0f};
    float v2[BLOCK] = {[0 ... (BLOCK-1)] = 0.0f};

    vec4f_t tmp_a = {0};
    vec4f_t tmp_b = {0};
    vec4f_t tmp_c = {0};

    for (size_t j = 0; j < BLOCK; j += 4) {
        vec4f_add(tmp_a, v1+j, v2+j);
        vec4f_add(tmp_b, tmp_a, tmp_b);
    }

    log_info("main", "%f, %f, %f, %f", tmp_b[0], tmp_b[1], tmp_b[2], tmp_b[3]);

    STATOCATOR(sa, _1KiB);
    ARRAY_INIT(int, &array, &sa);

    for (int i = 0; i < 100; ++i) {
        ARRAY_PUSH_BACK(int, &array, i);
    }

    for (int i = 0; i < ARRAY_SIZE(&array); ++i) {
        log_info("main", "II: %d", ARRAY_AT(&array, i));
    }

    //ARRAY_DESTROY(int, &array);

    STATOCATOR_CLEAR(&sa, _1KiB);

    quatf_t q1 = QUATF_IDENTITY;

    quatf_from_euler(q1, 1.0f, f_to_rad(180.0f), 0.0f);
    quatf_to_eurel_angle(v1, q1);

    log_info("main", "%f, %f, %f", v1[0], v1[1], v2[2]);

    void* yn = yaml_load_str(
        "foo: \n"
        "    bar:  2212\n"
        "    bars: true\n"
    );

    size_t foo = yaml_get_node(yn, 0, "foo");

    YAML_NODE_SCOPE(tmp_node, yn, foo, "bar", {
        log_info("main", "%u", yaml_node_as_int(yn, tmp_node));
    });

    YAML_NODE_SCOPE(tmp_node, yn, foo, "bars", {
        log_info("main", "%u", yaml_node_as_bool(yn, tmp_node));
    });

    log_shutdown();
    return 0;
}
