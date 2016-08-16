#include <include/SDL2/SDL.h>
#include <cetech/machine/types.h>

#define is_button_down(now, last) ((now) && !(last))
#define is_button_up(now, last)   (!(now) && (last))

void machine_process_keyboard_impl(struct eventstream *stream, u8 *last_state) {
    const u8 *state = SDL_GetKeyboardState(NULL);
    struct keyboard_event keyboard_ev;

    for (u32 i = 0; i < KEY_MAX; ++i) {
        if (is_button_down(state[i], last_state[i])) {
            keyboard_ev.keycode = i;
            event_stream_push(stream, EVENT_KEYBOARD_DOWN, keyboard_ev);

        } else if (is_button_up(state[i], last_state[i])) {
            keyboard_ev.keycode = i;
            event_stream_push(stream, EVENT_KEYBOARD_UP, keyboard_ev);

        }

        last_state[i] = state[i];
    }
}
