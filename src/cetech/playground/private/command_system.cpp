#include "celib/map.inl"

#include <cetech/debugui/debugui.h>
#include <cetech/playground/asset_property.h>
#include <cetech/resource/resource.h>
#include <cetech/playground/command_system.h>
#include <celib/array.h>

#include "cetech/hashlib/hashlib.h"
#include "cetech/config/config.h"
#include "cetech/os/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);

using namespace celib;

//TODO: MULTIPLE BUFFER (level view has own queue, identify by selected item);

#define _G asset_property_global
static struct _G {
    Map<ct_cmd_fce> cmd_map;
    uint8_t* cmd_buffer;
    uint32_t* cmd;
    uint32_t curent_pos;
    cel_alloc* allocator;
} _G;


static ct_cmd* get_curent_cmd() {
    if (!_G.curent_pos) {
        return NULL;
    }

    ct_cmd* curent_cmd = (ct_cmd*)&_G.cmd_buffer[_G.cmd[_G.curent_pos]];

    return curent_cmd;
}

void execute(const struct ct_cmd *cmd) {
    ct_cmd_fce cmd_fce = map::get<ct_cmd_fce>(_G.cmd_map, cmd->type, {NULL});

    if(!cmd_fce.execute) {
        return;
    }

    uint32_t buffer_offset = cel_array_size(_G.cmd_buffer);
    uint32_t size = cel_array_size(_G.cmd);

    if(_G.curent_pos != (size-1)) {
        ct_cmd* cur_cmd = get_curent_cmd();
        if(cur_cmd) {
//            uint32_t offset = _G.cmd[_G.curent_pos];
//            uint32_t end_offset = offset + cur_cmd->size;

//            array::resize(_G.cmd, _G.curent_pos + 1);
//            array::resize(_G.cmd_buffer, end_offset);
        }
    }

    cel_array_push_n(_G.cmd_buffer, (uint8_t*)cmd, cmd->size, _G.allocator);
    cel_array_push(_G.cmd, buffer_offset, _G.allocator);

    _G.curent_pos += 1;

    cmd_fce.execute(cmd, false);
}

void register_cmd_execute(uint64_t type, ct_cmd_fce fce) {
    map::set(_G.cmd_map, type, fce);
}

void undo() {
    ct_cmd* curent_cmd = get_curent_cmd();

    if(!curent_cmd) {
        return;
    }

    ct_cmd_fce cmd_fce = map::get<ct_cmd_fce>(_G.cmd_map, curent_cmd->type, {NULL});

    if(!cmd_fce.execute) {
        return;
    }

    cmd_fce.execute(curent_cmd, true);

    _G.curent_pos -= 1;
}


static ct_cmd* get_next_cmd() {
    uint32_t cmd_size = cel_array_size(_G.cmd);

    if (!cmd_size) {
        return NULL;
    }

    if(_G.curent_pos == (cmd_size-1)) {
        return NULL;
    }

    ct_cmd* next_cmd = (ct_cmd*)&_G.cmd_buffer[_G.cmd[_G.curent_pos + 1]];

    return next_cmd;
}

void redo() {
    ct_cmd* next_cmd = get_next_cmd();

    if(!next_cmd) {
        return;
    }

    ct_cmd_fce cmd_fce = map::get<ct_cmd_fce>(_G.cmd_map, next_cmd->type, {NULL});
    if(!cmd_fce.execute) {
        return;
    }

    cmd_fce.execute(next_cmd, false);

    _G.curent_pos += 1;
}

void undo_text(char* buffer, uint32_t buffer_size) {
    ct_cmd* curent_cmd = get_curent_cmd();

    if(!curent_cmd){
        buffer[0] = '\0';
        return;
    }

    ct_cmd_fce cmd_fce = map::get<ct_cmd_fce>(_G.cmd_map, curent_cmd->type, {NULL});
    if(!cmd_fce.description) {
        buffer[0] = '\0';
        return;
    }

    cmd_fce.description(buffer, buffer_size, curent_cmd, true);
}

void redo_text(char* buffer, uint32_t buffer_size) {
    ct_cmd* next_cmd = get_next_cmd();

    if(!next_cmd){
        buffer[0] = '\0';
        return;
    }

    ct_cmd_fce cmd_fce = map::get<ct_cmd_fce>(_G.cmd_map, next_cmd->type, {NULL});
    if(!cmd_fce.description) {
        buffer[0] = '\0';
        return;
    }

    cmd_fce.description(buffer, buffer_size, next_cmd, true);
}

void command_text(char* buffer, uint32_t buffer_size, uint32_t idx) {
    const struct ct_cmd* cmd = (const struct ct_cmd* ) &_G.cmd_buffer[_G.cmd[idx]];

    ct_cmd_fce cmd_fce = map::get<ct_cmd_fce>(_G.cmd_map, cmd->type, {NULL});
    if(!cmd_fce.description) {
        goto invalid;
    }

    cmd_fce.description(buffer, buffer_size, cmd, true);

    return;
    invalid:
    buffer[0] = '\0';
}

uint32_t command_count() {
    return cel_array_size(_G.cmd) - 1;
}

void goto_idx(uint32_t idx) {
    int diff = _G.curent_pos - idx;

    if(!diff) {
        return;
    }

    if( diff < 0 ){
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


static void _init(ct_api_a0 *api) {
    _G = {
            .curent_pos = 0,
            .allocator = ct_memory_a0.main_allocator(),
    };

    api->register_api("ct_cmd_system_a0", &cmd_system_a0);

    _G.cmd_map.init(ct_memory_a0.main_allocator());

    cel_array_push(_G.cmd, 0, _G.allocator);
}

static void _shutdown() {
    _G.cmd_map.destroy();

    cel_array_free(_G.cmd_buffer, _G.allocator);
    cel_array_free(_G.cmd, _G.allocator);

    _G = {};
}

CETECH_MODULE_DEF(
        cmd_system,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);

        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            _shutdown();
        }
)