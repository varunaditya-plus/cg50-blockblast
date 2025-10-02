#ifndef GRID_H
#define GRID_H

// Grid dimensions - 8x8 centered
#define GRID_SIZE 8
#define GRID_CELL_SIZE 20  // 20x20 pixel cells

// Grid colors (RGB565 format for CG-50)
#define COLOR_BACKGROUND 0x3270  // #364C87 converted to RGB565
#define COLOR_GRID_LINE 0x1907   // #1A1F3D converted to RGB565
#define COLOR_BLACK 0x0000       // Black in RGB565

// Grid position (centered on screen)
#define GRID_X_OFFSET 118  // (396 - 160) / 2
#define GRID_Y_OFFSET 32   // (224 - 160) / 2

// Grid state management
#define MAX_PLACED_BLOCKS 10
#define BLOCK_TYPE_EMPTY -1

// Scoring system
#define SCORE_PIECE_PLACEMENT 5
#define SCORE_LINE_CLEAR_1 20
#define SCORE_LINE_CLEAR_2 40
#define SCORE_LINE_CLEAR_3 80
#define SCORE_LINE_CLEAR_4_PLUS 140

typedef struct {
    int piece_type;
    int grid_x;
    int grid_y;
    int is_active;  // 1 if this block is currently being moved
} placed_block_t;

// Function declarations
void grid_init(void);
void grid_draw(void);
void grid_clear(void);
void grid_place_block(int piece_type, int grid_x, int grid_y);
void grid_move_active_block(int dx, int dy);
void grid_set_active_block(int block_index);
int grid_get_active_block(void);
void grid_draw_placed_blocks(void);
int grid_is_valid_position(int piece_type, int grid_x, int grid_y);
void grid_finalize_active_block(void);
int grid_active_overlaps_existing(void);
int grid_would_overlap(int piece_type, int grid_x, int grid_y);
int grid_can_place(int piece_type, int grid_x, int grid_y);
// Find first valid position for a piece; returns 1 if found and sets out_x/out_y
int grid_find_first_fit(int piece_type, int *out_x, int *out_y);
// Check if any piece can be placed anywhere on the grid
int grid_can_any_piece_fit(int available_pieces[], int num_pieces);
// Display GAME OVER text in the center of the screen
void grid_draw_game_over(void);
// Cancel active block placement and return piece type for restoration
int grid_cancel_active_block(void);
// Score management
int grid_get_score(void);
void grid_draw_score(void);

#endif // GRID_H
