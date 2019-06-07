#ifndef CETECH_DEBUGDRAW_H
#define CETECH_DEBUGDRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CT_DD_A0 \
    CE_ID64_0("ct_dd_a0", 0xfdd19bc6b2e51af8ULL)

typedef enum {
    CT_DD_AXIS_NONE,
    CT_DD_AXIS_X,
    CT_DD_AXIS_Y,
    CT_DD_AXIS_Z,
} ct_dd_axis_e0;

struct ct_dd_a0 {
    void (*begin)(uint8_t view);

    void (*end)();

    void (*push)();

    void (*pop)();

    void (*set_transform_mtx)(float *mtx);

    void (*set_color)(float r,
                      float g,
                      float b,
                      float a);

    void (*draw_axis)(float x,
                      float y,
                      float z,
                      float len,
                      ct_dd_axis_e0 highlight,
                      float thickness);

    void (*move_to)(float x,
                    float y,
                    float z);

    void (*line_to)(float x,
                    float y,
                    float z);

    void (*close)();
};

CE_MODULE(ct_dd_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_DEBUGDRAW_H
