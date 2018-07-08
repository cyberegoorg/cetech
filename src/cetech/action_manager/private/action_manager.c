#include <corelib/log.h>
#include <corelib/hashlib.h>
#include <corelib/memory.h>
#include <corelib/api_system.h>
#include <corelib/module.h>

#include <cetech/action_manager/action_manager.h>
#include <cetech/controlers/keyboard.h>
#include <corelib/array.inl>
#include <corelib/hash.inl>
#include <cetech/controlers/controlers.h>

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

    struct ct_controlers_i0* keyboard;
    keyboard = ct_controlers_a0->get_by_name(CT_ID64_0("keyboard"));

    for (int i = 0; i < count; ++i) {
        if (!strcmp(begin[i], "shift")) {
            sc->mod.flags.shift = 1;
        } else if (!strcmp(begin[i], "ctrl")) {
            sc->mod.flags.ctrl = 1;
        } else if (!strcmp(begin[i], "alt")) {
            sc->mod.flags.alt = 1;
        } else {
            sc->key = keyboard->button_index(begin[i]);
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
    struct ct_controlers_i0* keyboard;
    keyboard = ct_controlers_a0->get_by_name(CT_ID64_0("keyboard"));


    const uint32_t lshift = keyboard->button_index("lshift");
    const uint32_t rshift = keyboard->button_index("rshift");
    const uint32_t lctrl = keyboard->button_index("lctrl");
    const uint32_t rctrl = keyboard->button_index("rctrl");
    const uint32_t lalt = keyboard->button_index("lalt");
    const uint32_t ralt = keyboard->button_index("ralt");

    _G.mod = (union modifiactor) {};

    if (keyboard->button_state(0, lshift) ||
            keyboard->button_state(0, rshift)) {
        _G.mod.flags.shift = 1;
    }

    if (keyboard->button_state(0, lctrl) ||
            keyboard->button_state(0, rctrl)) {
        _G.mod.flags.ctrl = 1;
    }

    if (keyboard->button_state(0, lalt) ||
            keyboard->button_state(0, ralt)) {
        _G.mod.flags.alt = 1;
    }

    const int size = ct_array_size(_G.shorcut);
    for (int i = 0; i < size; ++i) {
        struct shortcut *sc = &_G.shorcut[i];

        if (_G.mod.u - sc->mod.u) {
            continue;
        }

        bool active = keyboard->button_state(0, sc->key) > 0;

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

struct ct_action_manager_a0 *ct_action_manager_a0 = &action_manager_api;

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0->system
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