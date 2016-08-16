
#include <celib/containers/array.h>
#include <cetech/machine/machine.h>

#define LOG_WHERE "machine"

static struct G {
    struct eventstream eventstream;

    u8 keyboard_last_state[KEY_MAX];
    u8 mouse_last_state[MOUSE_BTN_MAX];
    int mouse_last_position[2];
} _G = {0};

int machine_init_impl();

int machine_shutdown_impl();

void machine_process_keyboard_impl(struct eventstream *stream, u8 *last_state);

void machine_process_mouse_impl(struct eventstream *stream, u8 *last_state, int *last_pos);

void machine_process_impl(struct eventstream *stream);

int machine_init() {
    _G = (struct G) {0};

    eventstream_create(&_G.eventstream, memsys_main_allocator());

    return machine_init_impl();
}

void machine_shutdown() {
    eventstream_destroy(&_G.eventstream);

    _G = (struct G) {0};

    machine_shutdown_impl();
}

void machine_begin_frame() {
    machine_process_impl(&_G.eventstream);
    machine_process_keyboard_impl(&_G.eventstream, _G.keyboard_last_state);
    machine_process_mouse_impl(&_G.eventstream, _G.mouse_last_state, _G.mouse_last_position);
}

void machine_end_frame() {
    eventstream_clear(&_G.eventstream);
}

struct event_header *machine_event_begin() {
    return eventstream_begin(&_G.eventstream);
}

struct event_header *machine_event_end() {
    return eventstream_end(&_G.eventstream);
}

struct event_header *machine_event_next(struct event_header *header) {
    return eventstream_next(header);
}