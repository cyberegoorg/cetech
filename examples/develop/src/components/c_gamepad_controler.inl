#include <stdlib.h>

#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <celib/math/math.h>

// component

typedef struct gamepad_controler_component {
    float gamepad_id;
} gamepad_controler_component;

#define GAMEPAD_COMPONENT \
    CE_ID64_0("gamepad_controller", 0x49293316c5688da9ULL)

static uint64_t gamepad_controler_cdb_type() {
    return ce_id_a0->id64("gamepad_controller");
}

static const char *gamepad_controler_display_name() {
    return "Gamepad controler";
}

static uint64_t gamepad_controler_size() {
    return sizeof(gamepad_controler_component);
}

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
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(gamepad_controler_component));

}

static struct ct_ecs_component_i0 gamepad_controler_component_i = {
        .display_name = gamepad_controler_display_name,
        .cdb_type = gamepad_controler_cdb_type,
        .size = gamepad_controler_size,
        .on_spawn = _gamepad_controler_on_spawn,
        .on_change = _gamepad_controler_on_spawn,
};
