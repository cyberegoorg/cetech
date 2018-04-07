#include <cetech/kernel/log/log.h>
#include <cetech/kernel/hashlib/hashlib.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/module/module.h>

#include <cetech/playground/action_manager.h>
#include <cetech/engine/controlers/keyboard.h>
#include <cetech/kernel/containers/array.h>
#include <cetech/kernel/containers/hash.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_keyboard_a0);

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
    union modifiactor mod;
    uint32_t key;
};


#define _G action_manager_global
static struct _G {
    struct ct_hash_t action_map;
    union modifiactor mod;

    struct shortcut *shorcut;
    action_fce_t *action_fce;
    bool *action_active;
    struct ct_alloc *allocator;
} _G;

static void fill_button(struct shortcut *sc) {
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
    uint32_t idx = ct_array_size(_G.shorcut);
    struct shortcut sc = {};

    strncpy(sc.str, shortcut_str, 64);
    fill_button(&sc);

    ct_array_push_n(_G.shorcut, &sc, 1, _G.allocator);
    ct_array_push(_G.action_active, false, _G.allocator);
    ct_array_push(_G.action_fce, fce, _G.allocator);
    ct_hash_add(&_G.action_map, name, idx, _G.allocator);
}

static void unregister_action(uint64_t name) {

}

static void execute(uint64_t name) {
    uint32_t idx = ct_hash_lookup(&_G.action_map, name, UINT32_MAX);

    if (UINT32_MAX == idx) {
        return;
    }

    _G.action_fce[idx]();
}

static void check() {
    const uint32_t lshift = ct_keyboard_a0.button_index("lshift");
    const uint32_t rshift = ct_keyboard_a0.button_index("rshift");
    const uint32_t lctrl = ct_keyboard_a0.button_index("lctrl");
    const uint32_t rctrl = ct_keyboard_a0.button_index("rctrl");
    const uint32_t lalt = ct_keyboard_a0.button_index("lalt");
    const uint32_t ralt = ct_keyboard_a0.button_index("ralt");

    _G.mod = (union modifiactor) {};

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

    const int size = ct_array_size(_G.shorcut);
    for (int i = 0; i < size; ++i) {
        struct shortcut *sc = &_G.shorcut[i];

        if (_G.mod.u - sc->mod.u) {
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

const char *shortcut_str(uint64_t name) {
    uint32_t idx = ct_hash_lookup(&_G.action_map, name, UINT32_MAX);

    if (UINT32_MAX == idx) {
        return NULL;
    }


    return _G.shorcut[idx].str;
}

static struct ct_action_manager_a0 action_manager_api = {
        .register_action = register_action,
        .unregister_action = unregister_action,
        .execute = execute,
        .check = check,
        .shortcut_str = shortcut_str,
};


static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator()
    };

    api->register_api("ct_action_manager_a0", &action_manager_api);
}

static void _shutdown() {
    ct_hash_free(&_G.action_map, _G.allocator);

    ct_array_free(_G.shorcut, _G.allocator);
    ct_array_free(_G.action_fce, _G.allocator);
    ct_array_free(_G.action_active, _G.allocator);

    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        action_manager,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_keyboard_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)