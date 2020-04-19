
#include <cetech/property_editor/property_editor.h>
#include <celib/cdb_yaml.h>
#include <stdlib.h>
#include <celib/math/math.h>
#include <cetech/cdb_types/cdb_types.h>

// component
#define PLAYER_INPUT_COMPONENT \
    CE_ID64_0("player_input", 0xd89eea239dbc4d7aULL)

#define PLAYER_INPUT_MOVE \
    CE_ID64_0("move", 0x33603ac62788b5c5ULL)

#define PLAYER_INPUT_MOVE_SPEED \
    CE_ID64_0("move_speed", 0x992619d01515f950ULL)

#define PLAYER_INPUT_SHOOT_DIR \
    CE_ID64_0("shoot_dir", 0x2cd9f4c7b4921bb0ULL)


typedef struct player_input_component {
    float move;
    float move_speed;
    ce_vec2_t shoot_dir;
} player_input_component;


static void _player_input_on_spawn(ct_world_t0 world,
                                   ce_cdb_t0 db,
                                   uint64_t obj,
                                   void *data) {
    player_input_component *c = data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);

    uint64_t dir = ce_cdb_a0->read_subobject(r, PLAYER_INPUT_SHOOT_DIR, 0);
    const ce_cdb_obj_o0 *dir_r = ce_cdb_a0->read(db, dir);

    (*c) = (player_input_component) {
            .move = ce_cdb_a0->read_float(r, PLAYER_INPUT_MOVE, 0.0f),
            .move_speed = ce_cdb_a0->read_float(r, PLAYER_INPUT_MOVE_SPEED, 0.0f),
            .shoot_dir = (ce_vec2_t) {
                    .x = ce_cdb_a0->read_float(dir_r, PROP_VEC_X, 0.0f),
                    .y = ce_cdb_a0->read_float(dir_r, PROP_VEC_Y, 0.0f),
            }

    };
}

static struct ct_ecs_component_i0 player_input_component_i = {
        .display_name = "Player input",
        .cdb_type = PLAYER_INPUT_COMPONENT,
        .size = sizeof(player_input_component),
        .from_cdb_obj = _player_input_on_spawn,
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