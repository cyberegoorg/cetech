//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/eventstream.inl>
#include <cetech/core/module.h>
#include <cetech/core/memory.h>

#include <cetech/core/machine.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource.h>
#include <cetech/core/api.h>


using namespace cetech;

//==============================================================================
// Extern functions
//==============================================================================

extern int _machine_init_impl(struct api_v0 *api);

extern void _machine_shutdown_impl();

extern void _machine_process_impl(EventStream& stream);

extern int _machine_gamepad_is_active(int idx);

extern void _machine_gamepad_play_rumble(int gamepad,
                                         float strength,
                                         uint32_t length);

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "machine"


//==============================================================================
// Globals
//==============================================================================

static struct G {
    EventStream eventstream;
} _G = {0};

IMPORT_API(memory_api_v0);

//==============================================================================
// Interface
//==============================================================================

struct event_header *machine_event_begin() {
    return eventstream::begin(_G.eventstream);
}


struct event_header *machine_event_end() {
    return eventstream::end(_G.eventstream);
}

struct event_header *machine_event_next(struct event_header *header) {
    return eventstream::next(_G.eventstream, header);
}

static void _init_api(struct api_v0 *api) {
    static struct machine_api_v0 api_v1 = {
            .event_begin = machine_event_begin,
            .event_end = machine_event_end,
            .event_next = machine_event_next,
            .gamepad_is_active = _machine_gamepad_is_active,
            .gamepad_play_rumble = _machine_gamepad_play_rumble,
    };
    api->register_api("machine_api_v0", &api_v1);
}


static void _init(struct api_v0 *api) {
    GET_API(api, memory_api_v0);

    _G = {0};

    _G.eventstream.init(memory_api_v0.main_allocator());

    _machine_init_impl(api);
}

static void _shutdown() {
    _machine_shutdown_impl();

    _G = {0};
}

void _update() {
    eventstream::clear(_G.eventstream);
    _machine_process_impl(_G.eventstream);
}

extern "C" void *machine_get_module_api(int api) {

    if (api == PLUGIN_EXPORT_API_ID) {
        static struct module_export_api_v0 module = {0};

        module.init = _init;
        module.init_api = _init_api;
        module.shutdown = _shutdown;
        module.update = _update;

        return &module;

    }

    return 0;
}
