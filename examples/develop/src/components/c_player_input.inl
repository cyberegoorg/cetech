
#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <stdlib.h>
#include <celib/math/math.h>

// component
#define PLAYER_INPUT_MOVE \
    CE_ID64_0("move", 0x33603ac62788b5c5ULL)

typedef struct player_input_component {
    ce_vec2_t move;
} player_input_component;

#define PLAYER_INPUT_COMPONENT \
    CE_ID64_0("player_input", 0xd89eea239dbc4d7aULL)

static uint64_t player_input_cdb_type() {
    return PLAYER_INPUT_COMPONENT;
}

static const char *player_input_display_name() {
    return "Player input";
}

static void *player_input_get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = player_input_display_name,
        };
        return &ct_editor_component_i0;
    }

    return NULL;
}

static uint64_t player_input_size() {
    return sizeof(player_input_component);
}


static const ce_cdb_prop_def_t0 plyer_input_component_prop[] = {
        {
                .name = "move",
                .type = CDB_TYPE_SUBOBJECT,
                .obj_type = VEC2_CDB_TYPE,
                .flags = CDB_PROP_FLAG_UNPACK,
        },
};

static void _player_input_on_spawn(uint64_t obj,
                                   void *data) {
    player_input_component *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(player_input_component));
}

static struct ct_component_i0 player_input_component_i = {
        .cdb_type = player_input_cdb_type,
        .get_interface = player_input_get_interface,
        .size = player_input_size,
        .on_spawn = _player_input_on_spawn,
        .on_change = _player_input_on_spawn,
};

static const ce_cdb_prop_def_t0 player_input_component_prop[] = {
        {
                .name = "move",
                .type = CE_CDB_TYPE_FLOAT,
        },
        {
                .name = "shoot_dir",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC2_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};