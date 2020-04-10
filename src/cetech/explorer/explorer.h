#ifndef CETECH_EXPLORER_H
#define CETECH_EXPLORER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CT_EXPLORER_ASPECT \
    CE_ID64_0("explorer_aspect", 0xa92459da8af9d600ULL)


typedef uint64_t (ct_explorer_draw_ui_t)(uint64_t top_level_obj,
                                         uint64_t selected_obj,
                                         uint64_t context);

#ifdef __cplusplus
};
#endif

#endif //CETECH_EXPLORER_H
