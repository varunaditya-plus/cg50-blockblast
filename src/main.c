#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include "game_state.h"
#include "input_handler.h"
#include "renderer.h"
#include "grid.h"
#include "score.h"
#include "renderer.h"
#include "tetris_blocks.h"

int main(void)
{
    game_state_reset();
    // Ensure score file exists in calculator's main directory
    {
        const char *path = "/score.txt";
        FILE *fp = fopen(path, "a");
        if(fp) fclose(fp);
    }
    // Try to load the last score from /score.txt and display it
    {
        const char *path = "/score.txt";
        FILE *fp = fopen(path, "r");
        int last = -1;
        if(fp)
        {
            int value = 0;
            while(fscanf(fp, "%d", &value) == 1) { last = value; }
            fclose(fp);
        }
        if(last >= 0)
        {
            score_set_loaded(last);
        }
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
            int current = score_get_current();
            // Read latest saved score from file and only save if current > latest
            int last_saved = -1;
            {
                const char *path_r = "/score.txt";
                FILE *fr = fopen(path_r, "r");
                if(fr)
                {
                    int v = 0;
                    while(fscanf(fr, "%d", &v) == 1) { last_saved = v; }
                    fclose(fr);
                }
            }
            if(last_saved < 0 || current > last_saved)
            {
                const char *path_w = "/score.txt";
                FILE *fw = fopen(path_w, "a");
                if(fw)
                {
                    fprintf(fw, "%d\n", current);
                    fflush(fw);
                    fclose(fw);
                    // Update loaded to current so 'unsaved' disappears
                    score_set_loaded(current);
                    // Quick redraw of right panel to reflect changes
                    dclear(COLOR_BACKGROUND);
                    grid_draw();
                    grid_draw_placed_blocks();
                    grid_draw_score();
                    tetris_blocks_draw();
                    renderer_draw_footer();
                }
            }
        }
        
        dupdate();
    }
    
    return 0;
}
