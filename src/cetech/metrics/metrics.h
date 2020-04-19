#ifndef CETECH_METRICS_H
#define CETECH_METRICS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CT_METRICS_A0_STR "ct_metrics_a0"
#define CT_METRICS_API \
   CE_ID64_0("ct_metrics_a0", 0xcb85cc37c965043ULL)

struct ct_metrics_a0 {
    void (*begin)();

    void (*end)();

    void (*reg_float_metric)(const char *name);

    void (*clear_record)();

    void (*start_record)();

    void (*stop_record)();

    bool (*is_recording)();

    uint32_t (*recorded_frames_num)();

    void (*set_float)(uint64_t name,
                      float value);

    float (*get_float)(uint64_t name);

    const float *(*get_recorded_floats)(uint64_t name);
};

CE_MODULE(ct_metrics_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_METRICS_H