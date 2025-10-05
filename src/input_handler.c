#include <gint/display.h>
#include <gint/keyboard.h>
#include "input_handler.h"
#include "game_state.h"
#include "grid.h"
#include "tetris_blocks.h"
#include "renderer.h"

input_action_t input_handle_key(key_event_t key)
{
    switch (key.key)
    {
        case KEY_EXIT:
            return INPUT_ACTION_EXIT;
        case KEY_F1:
        case KEY_F2:
            return INPUT_ACTION_RESET;
        case KEY_EXE:
            return INPUT_ACTION_PLACE_BLOCK;
        case KEY_UP:
            return INPUT_ACTION_MOVE_UP;
        case KEY_DOWN:
            return INPUT_ACTION_MOVE_DOWN;
        case KEY_LEFT:
            return INPUT_ACTION_MOVE_LEFT;
        case KEY_RIGHT:
            return INPUT_ACTION_MOVE_RIGHT;
        default:
            return INPUT_ACTION_NONE;
    }
}

void input_process_action(input_action_t action)
{
    switch (action)
    {
        case INPUT_ACTION_EXIT:
            // If there's an active block being placed, cancel it and return piece to sidebar
            if (grid_get_active_block() != -1)
            {
                uint16_t color = grid_get_active_block_color();
                int piece_type = grid_cancel_active_block();
                if (piece_type >= 0)
                {
                    tetris_blocks_restore_piece_with_color(piece_type, color);
                }
                // Redraw everything after canceling
                dclear(COLOR_BACKGROUND);
                grid_draw();
                grid_draw_placed_blocks();
                grid_draw_score();
                tetris_blocks_draw();
                renderer_draw_footer();
                dupdate();
            }
            else
            {
                // Clean exit without calling game_reset to avoid display issues
                return;
            }
            break;
            
        case INPUT_ACTION_RESET:
            game_state_reset();
            break;
            
        case INPUT_ACTION_PLACE_BLOCK:
            if (grid_get_active_block() != -1)
            {
                // If active block overlaps an existing block, ignore EXE
                if (!grid_active_overlaps_existing())
                {
                    // Lock current active block in place (stays red)
                    grid_finalize_active_block();
                    // Regenerate pieces if needed after placing
                    tetris_blocks_regenerate_if_needed();
                }
            }
            else
            {
                // Spawn selected piece at top left corner and make it active
                int current_selection = tetris_blocks_get_selection();
                int piece_type = tetris_blocks_get_piece_type_for_selection(current_selection);
                if (piece_type < 0)
                {
                    // No available selection; try to regenerate if all consumed
                    tetris_blocks_regenerate_if_needed();
                    // Re-evaluate selection after potential regeneration
                    current_selection = tetris_blocks_get_selection();
                    piece_type = tetris_blocks_get_piece_type_for_selection(current_selection);
                    if (piece_type < 0)
                    {
                        // Still nothing to place
                        dclear(COLOR_BACKGROUND);
                        grid_draw();
                        grid_draw_placed_blocks();
                        grid_draw_score();
                        tetris_blocks_draw();
                        renderer_draw_footer();
                        dupdate();
                        return;
                    }
                }
                // Always place at top left corner (0, 0)
                int px = 0, py = 0;
                // Check if piece can fit at top left corner
                if (!grid_is_valid_position(piece_type, px, py))
                {
                    // Piece doesn't fit at top left, skip placement
                    dupdate();
                    return;
                }
                // Determine the color assigned to this selected slot
                uint16_t color = tetris_blocks_get_piece_color_for_slot(current_selection);
                grid_place_block(piece_type, px, py, color);
                // Consume the sidebar piece used
                tetris_blocks_consume_selected();
            }
            // Redraw everything
            dclear(COLOR_BACKGROUND);
            grid_draw();
            grid_draw_placed_blocks();
            grid_draw_score();
            tetris_blocks_draw();
            renderer_draw_footer();
            
            // Check for game over after piece placement
            game_state_check_game_over();
            break;
            
        case INPUT_ACTION_MOVE_UP:
            if (grid_get_active_block() != -1)
            {
                grid_move_active_block(0, -1);  // Move up
            }
            else
            {
                // Otherwise, change block selection
                int current_selection = tetris_blocks_get_selection();
                if (current_selection > 0)
                {
                    tetris_blocks_set_selection(current_selection - 1);
                }
            }
            // Redraw everything
            dclear(COLOR_BACKGROUND);
            grid_draw();
            grid_draw_placed_blocks();
            grid_draw_score();
            tetris_blocks_draw();
            renderer_draw_footer();
            break;
            
        case INPUT_ACTION_MOVE_DOWN:
            if (grid_get_active_block() != -1)
            {
                grid_move_active_block(0, 1);  // Move down
            }
            else
            {
                // Otherwise, change block selection
                int current_selection = tetris_blocks_get_selection();
                if (current_selection < 2)  // We have 3 blocks (0, 1, 2)
                {
                    tetris_blocks_set_selection(current_selection + 1);
                }
            }
            // Redraw everything
            dclear(COLOR_BACKGROUND);
            grid_draw();
            grid_draw_placed_blocks();
            grid_draw_score();
            tetris_blocks_draw();
            renderer_draw_footer();
            break;
            
        case INPUT_ACTION_MOVE_LEFT:
            if (grid_get_active_block() != -1)
            {
                grid_move_active_block(-1, 0);  // Move left
            }
            // Redraw everything
            dclear(COLOR_BACKGROUND);
            grid_draw();
            grid_draw_placed_blocks();
            grid_draw_score();
            tetris_blocks_draw();
            renderer_draw_footer();
            break;
            
        case INPUT_ACTION_MOVE_RIGHT:
            if (grid_get_active_block() != -1)
            {
                grid_move_active_block(1, 0);  // Move right
            }
            // Redraw everything
            dclear(COLOR_BACKGROUND);
            grid_draw();
            grid_draw_placed_blocks();
            grid_draw_score();
            tetris_blocks_draw();
            renderer_draw_footer();
            break;
            
        case INPUT_ACTION_NONE:
        default:
            break;
    }
}

// yeah i fucking know redrawing everything after a single action is bad but im lazy