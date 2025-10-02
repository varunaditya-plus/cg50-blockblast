#include <gint/display.h>
#include "game_state.h"
#include "grid.h"
#include "tetris_blocks.h"

// Game state
static int game_over = 0;

void game_state_init(void)
{
    game_over = 0;
}

void game_state_reset(void)
{
    // Clear the display buffer first
    dclear(COLOR_BACKGROUND);
    
    // Reset all game states
    grid_init();
    tetris_blocks_init();
    game_over = 0;
    
    // Redraw everything
    grid_draw();
    grid_draw_placed_blocks();
    grid_draw_score();
    tetris_blocks_draw();
    dupdate();
}

int game_state_is_over(void)
{
    return game_over;
}

void game_state_set_over(int is_over)
{
    game_over = is_over;
}

void game_state_check_game_over(void)
{
    int available_pieces[3];
    int piece_count;
    tetris_blocks_get_available_pieces(available_pieces, &piece_count);
    
    if (piece_count > 0 && !grid_can_any_piece_fit(available_pieces, piece_count))
    {
        game_over = 1;
    }
}
