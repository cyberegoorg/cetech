#ifndef CETECH_OBJECT_H
#define CETECH_OBJECT_H


#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Object
//==============================================================================

struct ct_object_a0 {
    void *(*load)(const char *path);

    void (*unload)(void *so);

    void *(*load_function)(void *so,
                           const char *name);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_OBJECT_H
