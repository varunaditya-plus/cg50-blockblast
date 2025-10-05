#ifndef TETRIS_BLOCKS_H
#define TETRIS_BLOCKS_H

#include <gint/display.h>

// Tetris block colors (RGB565 format for CG-50)
#define COLOR_TETRIS_RED 0xF800  // Red in RGB565
#define COLOR_TETRIS_WHITE 0xFFFF  // White in RGB565

// Tetris block dimensions (proportionate to grid)
#define TETRIS_BLOCK_SIZE 15  // Slightly smaller than grid cells for better proportion

// Tetris block position (to the left of the grid)
#define TETRIS_AREA_X 25   // Start position for tetris blocks (left side)
#define TETRIS_AREA_Y 15   // Vertical start position
#define TETRIS_SPACING 70  // Vertical spacing between blocks

// Tetris piece definitions (4x4 matrices)
#define TETRIS_PIECES 39

// Piece difficulty categories
typedef enum {
    PIECE_EASY = 0,    // Straights and blocks
    PIECE_MEDIUM = 1,  // Corners and L
    PIECE_HARD = 2,    // Zigzag and T
    PIECE_RARE = 3     // Special large pieces
} piece_difficulty_t;

// Difficulty weights (higher = more likely to spawn)
#define EASY_WEIGHT 10
#define MEDIUM_WEIGHT 5
#define HARD_WEIGHT 3
#define RARE_WEIGHT 1

// Function declarations
void tetris_blocks_init(void);
void tetris_blocks_draw(void);
void draw_tetris_piece(int x, int y, int piece_type, int is_selected);
void draw_tetris_piece_sized(int x, int y, int piece_type, int is_selected, int block_size, int gap);
int tetris_blocks_get_selection(void);
void tetris_blocks_set_selection(int selection);
int tetris_blocks_get_piece_type_for_selection(int selection);
// Get the RGB565 color for a sidebar piece slot. Returns default red
uint16_t tetris_blocks_get_piece_color_for_slot(int slot);
// Get the RGB565 color for a given piece type in the sidebar, or default
uint16_t tetris_blocks_get_color_for_piece_type(int piece_type);
int tetris_piece_cell(int piece_type, int row, int col);
void tetris_blocks_consume_selected(void);
void tetris_blocks_regenerate_if_needed(void);
// Get all available (non-consumed) pieces
void tetris_blocks_get_available_pieces(int pieces[], int *count);
// Restore a piece back to the sidebar (undo consumption)
void tetris_blocks_restore_piece(int piece_type);
// Restore a piece back with a specific color
void tetris_blocks_restore_piece_with_color(int piece_type, uint16_t color);
// Get piece difficulty category
piece_difficulty_t tetris_get_piece_difficulty(int piece_type);
// Check if a piece can be placed anywhere on the current grid
int tetris_piece_is_placeable(int piece_type);
// Generate weighted random piece based on difficulty
int tetris_generate_weighted_piece(void);
// Generate 3 new pieces with placeability validation and no duplicates
void tetris_generate_valid_pieces(void);

#endif // TETRIS_BLOCKS_H
