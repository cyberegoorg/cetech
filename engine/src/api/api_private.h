#ifndef CETECH__API_H
#define CETECH__API_H

struct cel_alloc;

namespace api {
    void init(struct cel_alloc *allocator);

    void shutdown();

    struct ct_api_a0 *v0();
};

#endif //CETECH__API_H
