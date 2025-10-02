#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <gint/keyboard.h>

// Input handling
typedef enum {
    INPUT_ACTION_NONE = 0,
    INPUT_ACTION_EXIT,
    INPUT_ACTION_RESET,
    INPUT_ACTION_PLACE_BLOCK,
    INPUT_ACTION_MOVE_UP,
    INPUT_ACTION_MOVE_DOWN,
    INPUT_ACTION_MOVE_LEFT,
    INPUT_ACTION_MOVE_RIGHT,
    INPUT_ACTION_SELECT_UP,
    INPUT_ACTION_SELECT_DOWN
} input_action_t;

input_action_t input_handle_key(key_event_t key);
void input_process_action(input_action_t action);

#endif // INPUT_HANDLER_H
