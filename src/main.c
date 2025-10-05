#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include "game_state.h"
#include "input_handler.h"
#include "renderer.h"
#include "grid.h"
#include "score.h"
#include "renderer.h"

int main(void)
{
    game_state_reset();
    // Ensure score file exists in calculator's main directory
    {
        const char *path = "/score.txt";
        FILE *fp = fopen(path, "a");
        if(fp) fclose(fp);
    }
    
    renderer_redraw_all();
    
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
            renderer_draw_footer();
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
        
        // On F6, write the current score to /score.txt
        if(key.key == KEY_F6)
        {
            const char *path = "/score.txt";
            FILE *fp = fopen(path, "a");
            if(fp)
            {
                fprintf(fp, "%d\n", score_get_current());
                fflush(fp);
                fclose(fp);
            }
        }
        
        dupdate();
    }
    
    return 0;
}
