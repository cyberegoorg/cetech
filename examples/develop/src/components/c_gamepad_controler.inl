#include <stdlib.h>

#include <cetech/property_editor/property_editor.h>
#include <celib/yaml_cdb.h>
#include <celib/math/math.h>

// component

typedef struct gamepad_controler_component {
    float gamepad_id;
} gamepad_controler_component;

#define GAMEPAD_COMPONENT \
    CE_ID64_0("gamepad_controller", 0x49293316c5688da9ULL)

#define GAMEPAD_ID \
    CE_ID64_0("gamepad_id", 0xc5ea6dc1179d6f0aULL)

static const ce_cdb_prop_def_t0 gamepad_controler_compo_prop[] = {
        {
                .name = "gamepad_id",
                .type = CE_CDB_TYPE_FLOAT,
        },
};

static void _gamepad_controler_on_spawn(ct_world_t0 world,
                                        ce_cdb_t0 db,
                                        uint64_t obj,
                                        void *data) {
    gamepad_controler_component *c = data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    (*c) = (gamepad_controler_component) {.gamepad_id = ce_cdb_a0->read_uint64(r, GAMEPAD_ID, 0)};

}

static struct ct_ecs_component_i0 gamepad_controler_component_i = {
        .display_name = "Gamepad controler",
        .cdb_type = GAMEPAD_COMPONENT,
        .size = sizeof(gamepad_controler_component),
        .from_cdb_obj = _gamepad_controler_on_spawn,
};
