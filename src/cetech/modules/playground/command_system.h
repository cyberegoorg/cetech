#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

//==============================================================================
// Api
//==============================================================================

typedef void (*ct_cmd_execute_t)(const struct ct_cmd *cmd,
                                 bool inverse);

struct ct_cmd {
    uint64_t type;
    uint32_t size;
    char description[52];
};

//! Command system API V0
struct ct_cmd_system_a0 {
    void (*execute)(const struct ct_cmd *cmd);

    void (*register_cmd_execute)(uint64_t type,
                                 ct_cmd_execute_t execute);

    void (*undo)();

    void (*redo)();

    const char *(*undo_text)();

    const char *(*redo_text)();
};

#ifdef __cplusplus
}
#endif

#endif //COMMAND_SYSTEM_H
