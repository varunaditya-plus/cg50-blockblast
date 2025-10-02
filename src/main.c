#include <gint/display.h>
#include <gint/keyboard.h>
#include "game_state.h"
#include "input_handler.h"
#include "renderer.h"
#include "grid.h"

int main(void)
{
    game_state_reset();
    
    while(1)
    {
        // if gameover show game over screen and wait for reset which ISNT FUCKING WORKING
        if (game_state_is_over())
        {
            dclear(COLOR_BACKGROUND);
            grid_draw();
            grid_draw_placed_blocks();
            grid_draw_score();
            renderer_draw_game_over();
            dupdate();
            
            // Wait for key press
            key_event_t key = getkey();
            
            // leave the game
            if(key.key == KEY_MENU)
            {
                return 0;
            }
            
            // reset the game
            if(key.key == KEY_F1 || key.key == KEY_F2)
            {
                game_state_reset();
            }
            continue;
        }
        
        // wait for key press
        key_event_t key = getkey();
        
        // process the input
        input_action_t action = input_handle_key(key);
        input_process_action(action);
        
        dupdate();
    }
    
    return 0;
}
