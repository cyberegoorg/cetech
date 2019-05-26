
#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <stdlib.h>
#include <celib/math/math.h>

// component
#define PLAYER_INPUT_COMPONENT \
    CE_ID64_0("player_input", 0xd89eea239dbc4d7aULL)

typedef struct player_input_component {
    float move;
    float move_speed;
    ce_vec2_t shoot_dir;
} player_input_component;

static uint64_t player_input_cdb_type() {
    return ce_id_a0->id64("player_input");
}

static const char *player_input_display_name() {
    return "Player input";
}

static uint64_t player_input_size() {
    return sizeof(player_input_component);
}

static void _player_input_on_spawn(uint64_t obj,
                                   void *data) {
    player_input_component *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(player_input_component));
}

static struct ct_ecs_component_i0 player_input_component_i = {
        .display_name = player_input_display_name,
        .cdb_type = player_input_cdb_type,
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
                .name = "move_speed",
                .type = CE_CDB_TYPE_FLOAT,
        },
        {
                .name = "shoot_dir",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC2_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};