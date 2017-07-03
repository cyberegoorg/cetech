//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/eventstream.inl>
#include <cetech/kernel/module.h>
#include <cetech/kernel/memory.h>

#include <cetech/kernel/machine.h>
#include <cetech/kernel/config.h>
#include <cetech/modules/resource.h>
#include <cetech/kernel/api.h>


using namespace cetech;

//==============================================================================
// Extern functions
//==============================================================================

extern int _machine_init_impl(struct api_v0 *api);

extern void _machine_shutdown_impl();

extern void _machine_process_impl(EventStream &stream);

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

#define _G MachineGlobals
static struct MachineGlobals {
    EventStream eventstream;
} MachineGlobals;

CETECH_DECL_API(memory_api_v0);

//==============================================================================
// Interface
//==============================================================================
namespace machine {
    struct event_header *machine_event_begin() {
        return eventstream::begin(_G.eventstream);
    }


    struct event_header *machine_event_end() {
        return eventstream::end(_G.eventstream);
    }

    struct event_header *machine_event_next(struct event_header *header) {
        return eventstream::next(_G.eventstream, header);
    }

    void _update() {
        eventstream::clear(_G.eventstream);
        _machine_process_impl(_G.eventstream);
    }

}

namespace machine_module {
    static struct machine_api_v0 api_v1 = {
            .event_begin = machine::machine_event_begin,
            .event_end = machine::machine_event_end,
            .event_next = machine::machine_event_next,
            .gamepad_is_active = _machine_gamepad_is_active,
            .gamepad_play_rumble = _machine_gamepad_play_rumble,
            .update = machine::_update,
    };


    static void _init(struct api_v0 *api) {
        api->register_api("machine_api_v0", &api_v1);

        CETECH_GET_API(api, memory_api_v0);

        _G = {0};

        _G.eventstream.init(memory_api_v0.main_allocator());

        _machine_init_impl(api);
    }

    static void _shutdown() {
        _machine_shutdown_impl();

        _G.eventstream.destroy();
    }

    extern "C" void *machine_load_module(struct api_v0 *api) {
        _init(api);
        return nullptr;

//        if (api == PLUGIN_EXPORT_API_ID) {
//            static struct module_export_api_v0 module = {0};
//
//            module.init = _init;
//            module.shutdown = _shutdown;
//
//            return &module;
//
//        }
//
//        return 0;
    }

    extern "C" void machine_unload_module(struct api_v0 *api) {
        _shutdown();
    }

}