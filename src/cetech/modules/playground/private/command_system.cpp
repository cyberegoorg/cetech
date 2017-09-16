#include "celib/map.inl"

#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/playground/asset_property.h>
#include <cetech/kernel/resource.h>
#include <cetech/modules/playground/command_system.h>

#include "cetech/kernel/hashlib.h"
#include "cetech/kernel/config.h"
#include "cetech/kernel/memory.h"
#include "cetech/kernel/api_system.h"
#include "cetech/kernel/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);

using namespace celib;

//TODO: MULTIPLE BUFFER (level view has own queue, identify by selected item);

#define _G asset_property_global
static struct _G {
    Map<ct_cmd_execute_t> cmd_map;
    Array<uint8_t> cmd_buffer;
    Array<uint32_t > cmd;
    uint32_t curent_pos;
} _G;


static ct_cmd* get_curent_cmd() {
    if (!_G.curent_pos) {
        return NULL;
    }

    ct_cmd* curent_cmd = (ct_cmd*)&_G.cmd_buffer[_G.cmd[_G.curent_pos]];

    return curent_cmd;
}

void execute(const struct ct_cmd *cmd) {
    ct_cmd_execute_t cmd_fce = map::get<ct_cmd_execute_t>(_G.cmd_map, cmd->type, NULL);

    if(!cmd_fce) {
        return;
    }

    uint32_t buffer_offset = array::size(_G.cmd_buffer);
    uint32_t size = array::size(_G.cmd);

    if(_G.curent_pos != (size-1)) {
        ct_cmd* cur_cmd = get_curent_cmd();
        if(cur_cmd) {
            uint32_t offset = _G.cmd[_G.curent_pos];
            uint32_t end_offset = offset + cur_cmd->size;

            array::resize(_G.cmd, _G.curent_pos + 1);
            array::resize(_G.cmd_buffer, end_offset);
        }
    }

    array::push<uint8_t>(_G.cmd_buffer, (uint8_t*)cmd, cmd->size);
    array::push_back<uint32_t >(_G.cmd, buffer_offset);

    _G.curent_pos += 1;

    cmd_fce(cmd, false);
}

void register_cmd_execute(uint64_t type, ct_cmd_execute_t execute) {
    map::set(_G.cmd_map, type, execute);
}


void undo() {
    ct_cmd* curent_cmd = get_curent_cmd();

    if(!curent_cmd) {
        return;
    }

    ct_cmd_execute_t cmd_fce = map::get<ct_cmd_execute_t>(_G.cmd_map, curent_cmd->type, NULL);

    if(!cmd_fce) {
        return;
    }
    cmd_fce(curent_cmd, true);

    _G.curent_pos -= 1;
}


static ct_cmd* get_next_cmd() {
    uint32_t cmd_size = array::size(_G.cmd);

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

    ct_cmd_execute_t cmd_fce = map::get<ct_cmd_execute_t>(_G.cmd_map, next_cmd->type, NULL);
    if(!cmd_fce) {
        return;
    }

    cmd_fce(next_cmd, false);

    _G.curent_pos += 1;
}

const char* undo_text() {
    ct_cmd* curent_cmd = get_curent_cmd();

    if(!curent_cmd){
        return NULL;
    }

    return curent_cmd->description;
}

const char* redo_text() {
    ct_cmd* next_cmd = get_next_cmd();

    if(!next_cmd){
        return NULL;
    }

    return next_cmd->description;
}


static struct ct_cmd_system_a0 cmd_system_a0 = {
        .execute = execute,
        .register_cmd_execute = register_cmd_execute,
        .undo = undo,
        .redo = redo,
        .undo_text = undo_text,
        .redo_text = redo_text,
};


static void _init(ct_api_a0 *api) {
    _G = {
            .curent_pos = 0
    };

    api->register_api("ct_cmd_system_a0", &cmd_system_a0);

    _G.cmd_map.init(ct_memory_a0.main_allocator());
    _G.cmd_buffer.init(ct_memory_a0.main_allocator());
    _G.cmd.init(ct_memory_a0.main_allocator());

    array::push_back<uint32_t >(_G.cmd, 0);
}

static void _shutdown() {
    _G.cmd_map.destroy();
    _G.cmd_buffer.destroy();
    _G.cmd.destroy();

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