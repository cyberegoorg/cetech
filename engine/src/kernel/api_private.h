#ifndef CETECH__API_H
#define CETECH__API_H

struct ct_allocator;

namespace api {
    void init(struct ct_allocator *allocator);

    void shutdown();

    struct ct_api_v0 *v0();
};

#endif //CETECH__API_H
