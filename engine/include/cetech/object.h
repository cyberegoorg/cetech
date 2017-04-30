#ifndef CETECH_OBJECT_H
#define CETECH_OBJECT_H

enum {
    OBJECT_API_ID = 2223344
};


struct object_api_v0 {
    void *(*load)(const char *path);

    void (*unload)(void *so);

    void *(*load_function)(void *so,
                               void *name);
};

#endif //CETECH_OBJECT_H
