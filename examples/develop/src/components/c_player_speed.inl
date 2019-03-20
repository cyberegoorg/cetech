
#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <stdlib.h>
#include <celib/math/math.h>

// component
#define PLAYER_SPEED \
    CE_ID64_0("move", 0x33603ac62788b5c5ULL)

typedef struct player_speed_component {
    float speed;
} player_speed_component;

#define PLAYER_SPEED_COMPONENT \
    CE_ID64_0("player_speed", 0xdbd3311dd03de8a5ULL)

static uint64_t player_speed_cdb_type() {
    return PLAYER_SPEED_COMPONENT;
}

static const char *player_speed_display_name() {
    return "Player speed";
}

static void *player_speed_get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = player_speed_display_name,
        };
        return &ct_editor_component_i0;
    }

    return NULL;
}

static uint64_t player_speed_size() {
    return sizeof(player_speed_component);
}


static const ce_cdb_prop_def_t0 plyer_speed_component_prop[] = {
        {
                .name = "speed",
                .type = CDB_TYPE_FLOAT,
        },
};

static void _player_speed_on_spawn(uint64_t obj,
                                   void *data) {
    player_speed_component *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(player_speed_component));
}

static struct ct_component_i0 player_speed_component_i = {
        .cdb_type = player_speed_cdb_type,
        .get_interface = player_speed_get_interface,
        .size = player_speed_size,
        .on_spawn = _player_speed_on_spawn,
        .on_change = _player_speed_on_spawn,
};
