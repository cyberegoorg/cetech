#ifndef CETECH__API_H
#define CETECH__API_H

struct allocator;

namespace api {
    void init(struct allocator *allocator);

    void shutdown();

    struct api_v0 *v0();
};


#endif //CETECH__API_H
