#include "celib/map.inl"

#include <cetech/log/log.h>
#include <cetech/hashlib/hashlib.h>
#include <cetech/os/memory.h>
#include <cetech/api/api_system.h>
#include <cetech/module/module.h>

#include <cetech/playground/action_manager.h>
#include <cetech/input/input.h>
#include <celib/array.h>
#include <celib/hash.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_keyboard_a0);

using namespace celib;

typedef void (*action_fce_t)();

union modifiactor {
    uint8_t u;
    struct {
        uint8_t ctrl:1;
        uint8_t alt:1;
        uint8_t shift:1;
    } flags;
};

struct shortcut {
    char str[64];
    modifiactor mod;
    uint32_t key;
};


#define _G action_manager_global
static struct _G {
    cel_hash_t action_map;
    modifiactor mod;

    shortcut* shorcut;
    action_fce_t* action_fce;
    bool* action_active;
    cel_alloc* allocator;
} _G;

static void fill_button(shortcut *sc) {
    uint8_t count = 1;
    char str[64] = {};
    strncpy(str, sc->str, 64);

    char *it = str;
    const char *begin[4] = {str};
    while (*it != '\0') {
        if (*it == '+') {
            begin[count] = it + 1;
            *it = '\0';
            ++count;
        }
        ++it;
    }

    for (int i = 0; i < count; ++i) {
        if (!strcmp(begin[i], "shift")) {
            sc->mod.flags.shift = 1;
        } else if (!strcmp(begin[i], "ctrl")) {
            sc->mod.flags.ctrl = 1;
        } else if (!strcmp(begin[i], "alt")) {
            sc->mod.flags.alt = 1;
        } else {
            sc->key = ct_keyboard_a0.button_index(begin[i]);
        }
    }
}

static void register_action(uint64_t name,
                            const char *shortcut_str,
                            action_fce_t fce) {
    uint32_t idx = cel_array_size(_G.shorcut);
    shortcut sc = {};

    strncpy(sc.str, shortcut_str, 64);
    fill_button(&sc);

    cel_array_push_n(_G.shorcut, &sc, 1, _G.allocator);
    cel_array_push(_G.action_active, false, _G.allocator);
    cel_array_push(_G.action_fce, fce, _G.allocator);
    cel_hash_add(&_G.action_map, name, idx, _G.allocator);
}

static void unregister_action(uint64_t name) {

}

static void execute(uint64_t name) {
    uint32_t idx = cel_hash_lookup(&_G.action_map, name, UINT32_MAX);

    if (UINT32_MAX == idx) {
        return;
    }

    _G.action_fce[idx]();
}

static void check() {
    static const uint32_t lshift = ct_keyboard_a0.button_index("lshift");
    static const uint32_t rshift = ct_keyboard_a0.button_index("rshift");
    static const uint32_t lctrl = ct_keyboard_a0.button_index("lctrl");
    static const uint32_t rctrl = ct_keyboard_a0.button_index("rctrl");
    static const uint32_t lalt = ct_keyboard_a0.button_index("lalt");
    static const uint32_t ralt = ct_keyboard_a0.button_index("ralt");

    _G.mod = {};

    if (ct_keyboard_a0.button_state(0, lshift) ||
        ct_keyboard_a0.button_state(0, rshift)) {
        _G.mod.flags.shift = 1;
    }

    if (ct_keyboard_a0.button_state(0, lctrl) ||
        ct_keyboard_a0.button_state(0, rctrl)) {
        _G.mod.flags.ctrl = 1;
    }

    if (ct_keyboard_a0.button_state(0, lalt) ||
        ct_keyboard_a0.button_state(0, ralt)) {
        _G.mod.flags.alt = 1;
    }

    const int size = cel_array_size(_G.shorcut);
    for (int i = 0; i < size; ++i) {
        shortcut *sc = &_G.shorcut[i];

        if( _G.mod.u - sc->mod.u ) {
            continue;
        }

        bool active = ct_keyboard_a0.button_state(0, sc->key) > 0;

        if (active && !_G.action_active[i]) {
            _G.action_fce[i]();
            _G.action_active[i] = true;
        }

        if (!active && _G.action_active[i]) {
            _G.action_active[i] = false;
        }
    }
}

const char* shortcut_str(uint64_t name) {
    uint32_t idx = cel_hash_lookup(&_G.action_map, name, UINT32_MAX);

    if (UINT32_MAX == idx) {
        return NULL;
    }


    return _G.shorcut[idx].str;
}

static ct_action_manager_a0 action_manager_api = {
        .register_action = register_action,
        .unregister_action = unregister_action,
        .execute = execute,
        .check = check,
        .shortcut_str = shortcut_str,
};


static void _init(ct_api_a0 *api) {
    _G = {
        .allocator = ct_memory_a0.main_allocator()
    };

    api->register_api("ct_action_manager_a0", &action_manager_api);
}

static void _shutdown() {
    cel_hash_free(&_G.action_map, _G.allocator);

    cel_array_free(_G.shorcut, _G.allocator);
    cel_array_free(_G.action_fce, _G.allocator);
    cel_array_free(_G.action_active, _G.allocator);

    _G = {};
}

CETECH_MODULE_DEF(
        action_manager,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_keyboard_a0);
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