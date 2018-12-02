#include <cetech/resource/resource.h>
#include <cetech/command_system/command_system.h>
#include <celib/array.inl>
#include <celib/hash.inl>
#include <stdio.h>

#include "celib/hashlib.h"
#include "celib/config.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"


//TODO: MULTIPLE BUFFER (level view has own queue, identify by selected item);

#define _G asset_property_global
static struct _G {
    struct ce_hash_t cmd_map;
    struct ct_cmd_fce *cmds;

    uint8_t *cmd_buffer;
    uint32_t *cmd;
    uint32_t curent_pos;
    struct ce_alloc *allocator;
} _G;


static struct ct_cmd *get_curent_cmd() {
    if (!_G.curent_pos) {
        return NULL;
    }

    struct ct_cmd *curent_cmd = (struct ct_cmd *) &_G.cmd_buffer[_G.cmd[_G.curent_pos]];

    return curent_cmd;
}

void execute(const struct ct_cmd *cmd) {
    uint32_t idx = ce_hash_lookup(&_G.cmd_map, cmd->type, UINT32_MAX);
    struct ct_cmd_fce cmd_fce = (UINT32_MAX != idx ? _G.cmds[idx]
                                                   : (struct ct_cmd_fce) {
                    NULL});

    if (!cmd_fce.execute) {
        return;
    }

    uint32_t buffer_offset = ce_array_size(_G.cmd_buffer);
    uint32_t end = ce_array_size(_G.cmd)-1;

    if (_G.curent_pos != end) {
        struct ct_cmd *cur_cmd = get_curent_cmd();
        if (cur_cmd) {
            uint32_t offset = _G.cmd[_G.curent_pos];
            uint32_t end_offset = offset + cur_cmd->size;

            ce_array_resize(_G.cmd, _G.curent_pos + 1, _G.allocator);
            ce_array_resize(_G.cmd_buffer, end_offset, _G.allocator);
        }
    } else {
        ce_array_push_n(_G.cmd_buffer, (uint8_t *) cmd, cmd->size, _G.allocator);
        ce_array_push(_G.cmd, buffer_offset, _G.allocator);
    }

    _G.curent_pos += 1;

    cmd_fce.execute(cmd, false);
}

void register_cmd_execute(uint64_t type,
                          struct ct_cmd_fce fce) {
    ce_array_push(_G.cmds, fce, _G.allocator);
    ce_hash_add(&_G.cmd_map, type, ce_array_size(_G.cmds) - 1, _G.allocator);
}

void undo() {
    struct ct_cmd *curent_cmd = get_curent_cmd();

    if (!curent_cmd) {
        return;
    }

    uint32_t idx = ce_hash_lookup(&_G.cmd_map, curent_cmd->type, UINT32_MAX);
    struct ct_cmd_fce cmd_fce = (UINT32_MAX != idx ? _G.cmds[idx]
                                                   : (struct ct_cmd_fce) {
                    NULL});

    if (!cmd_fce.execute) {
        return;
    }

    cmd_fce.execute(curent_cmd, true);

    _G.curent_pos -= 1;
}


static struct ct_cmd *get_next_cmd() {
    uint32_t cmd_size = ce_array_size(_G.cmd);

    if (!cmd_size) {
        return NULL;
    }

    if (_G.curent_pos == (cmd_size - 1)) {
        return NULL;
    }

    struct ct_cmd *next_cmd = (struct ct_cmd *) &_G.cmd_buffer[_G.cmd[
            _G.curent_pos + 1]];

    return next_cmd;
}

void redo() {
    struct ct_cmd *next_cmd = get_next_cmd();

    if (!next_cmd) {
        return;
    }

    uint32_t idx = ce_hash_lookup(&_G.cmd_map, next_cmd->type, UINT32_MAX);
    struct ct_cmd_fce cmd_fce = (UINT32_MAX != idx ? _G.cmds[idx]
                                                   : (struct ct_cmd_fce) {
                    NULL});

    if (!cmd_fce.execute) {
        return;
    }

    cmd_fce.execute(next_cmd, false);

    _G.curent_pos += 1;
}

void undo_text(char *buffer,
               uint32_t buffer_size) {
    struct ct_cmd *curent_cmd = get_curent_cmd();

    if (!curent_cmd) {
        buffer[0] = '\0';
        return;
    }

    uint32_t idx = ce_hash_lookup(&_G.cmd_map, curent_cmd->type, UINT32_MAX);
    struct ct_cmd_fce cmd_fce = (UINT32_MAX != idx ? _G.cmds[idx]
                                                   : (struct ct_cmd_fce) {
                    NULL});

    if (!cmd_fce.description) {
        buffer[0] = '\0';
        return;
    }

    cmd_fce.description(buffer, buffer_size, curent_cmd, true);

    if(!strlen(buffer)) {
        snprintf(buffer, buffer_size, "%s", "<no description>");
    }

}

void redo_text(char *buffer,
               uint32_t buffer_size) {
    struct ct_cmd *next_cmd = get_next_cmd();

    if (!next_cmd) {
        buffer[0] = '\0';
        return;
    }

    uint32_t idx = ce_hash_lookup(&_G.cmd_map, next_cmd->type, UINT32_MAX);
    struct ct_cmd_fce cmd_fce = (UINT32_MAX != idx ? _G.cmds[idx]
                                                   : (struct ct_cmd_fce) {
                    NULL});

    if (!cmd_fce.description) {
        buffer[0] = '\0';
        return;
    }

    cmd_fce.description(buffer, buffer_size, next_cmd, true);
}

void command_text(char *buffer,
                  uint32_t buffer_size,
                  uint32_t idx) {
    const struct ct_cmd *cmd = (const struct ct_cmd *) &_G.cmd_buffer[_G.cmd[idx]];

    uint32_t idxx = ce_hash_lookup(&_G.cmd_map, cmd->type, UINT32_MAX);
    struct ct_cmd_fce cmd_fce = (UINT32_MAX != idxx ? _G.cmds[idxx]
                                                    : (struct ct_cmd_fce) {
                    NULL});

    if (!cmd_fce.description) {
        buffer[0] = '\0';
        return;
    }

    cmd_fce.description(buffer, buffer_size, cmd, true);
}

uint32_t command_count() {
    return ce_array_size(_G.cmd) - 1;
}

void goto_idx(uint32_t idx) {
    int diff = _G.curent_pos - idx;

    if (!diff) {
        return;
    }

    if (diff < 0) {
        diff = -diff;
        for (int i = 0; i < diff; ++i) {
            redo();
        }
    } else {
        for (int i = 0; i < diff; ++i) {
            undo();
        }
    }
}

uint32_t curent_idx() {
    return _G.curent_pos;
}

static struct ct_cmd_system_a0 cmd_system_a0 = {
        .execute = execute,
        .register_cmd_execute = register_cmd_execute,
        .undo = undo,
        .redo = redo,
        .undo_text = undo_text,
        .redo_text = redo_text,
        .command_text = command_text,
        .command_count= command_count,
        .curent_idx = curent_idx,
        .goto_idx = goto_idx,
};


struct ct_cmd_system_a0 *ct_cmd_system_a0 = &cmd_system_a0;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .curent_pos = 0,
            .allocator = ce_memory_a0->system,
    };

    api->register_api("ct_cmd_system_a0", &cmd_system_a0);

    ce_array_push(_G.cmd, 0, _G.allocator);
}

static void _shutdown() {
    ce_array_free(_G.cmds, _G.allocator);
    ce_hash_free(&_G.cmd_map, _G.allocator);

    ce_array_free(_G.cmd_buffer, _G.allocator);
    ce_array_free(_G.cmd, _G.allocator);

    _G = (struct _G) {};
}

CE_MODULE_DEF(
        cmd_system,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);

        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)