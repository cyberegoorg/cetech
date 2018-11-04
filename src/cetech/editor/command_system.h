#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

#include <stdint.h>
#include <stddef.h>

#include <celib/module.inl>

struct ct_cmd {
    uint64_t type;
    uint32_t size;
};

struct ct_cdb_cmd_s {
    struct ct_cmd header;
    uint64_t cmd;
};


typedef void (*ct_cmd_execute_t)(const struct ct_cmd *cmd,
                                 bool inverse);

typedef void (*ct_cmd_description_t)(char *buffer,
                                     uint32_t buffer_size,
                                     const struct ct_cmd *cmd,
                                     bool inverse);

struct ct_cmd_fce {
    ct_cmd_execute_t execute;
    ct_cmd_description_t description;
};

struct ct_cmd_system_a0 {
    void (*execute)(const struct ct_cmd *cmd);

    void (*register_cmd_execute)(uint64_t type,
                                 struct ct_cmd_fce fce);

    void (*undo)();

    void (*redo)();

    void (*undo_text)(char *buffer,
                      uint32_t buffer_size);

    void (*redo_text)(char *buffer,
                      uint32_t buffer_size);

    void (*command_text)(char *buffer,
                         uint32_t buffer_size,
                         uint32_t idx);

    uint32_t (*command_count)();

    uint32_t (*curent_idx)();

    void (*goto_idx)(uint32_t idx);
};

CE_MODULE(ct_cmd_system_a0);

#endif //COMMAND_SYSTEM_H
