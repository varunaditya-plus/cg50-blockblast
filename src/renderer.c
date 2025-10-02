#include <gint/display.h>
#include "renderer.h"
#include "font.h"
#include "grid.h"
#include "tetris_blocks.h"

// Screen dimensions
#define SCREEN_WIDTH 396
#define SCREEN_HEIGHT 224

// Grid colors (RGB565 format for CG-50)
#define COLOR_BACKGROUND 0x3270  // #364C87 converted to RGB565
#define COLOR_TETRIS_RED 0xF800  // Red in RGB565
#define COLOR_BLACK 0x0000       // Black in RGB565

// Grid position (centered on screen)
#define GRID_X_OFFSET 118  // (396 - 160) / 2
#define GRID_Y_OFFSET 32   // (224 - 160) / 2
#define GRID_SIZE 8
#define GRID_CELL_SIZE 20  // 20x20 pixel cells

void renderer_draw_game_over(void)
{
    // Calculate center position for "GAME OVER" text
    int text_width = 9 * 8; // "GAME OVER" = 9 characters * 8 pixels each
    int text_height = 8; // Font height
    int center_x = (SCREEN_WIDTH - text_width) / 2;
    int center_y = (SCREEN_HEIGHT - text_height) / 2;
    
    // Draw "GAME OVER" text
    font_draw_text(center_x, center_y, "GAME OVER. PRESS F1 TO RESET.");
}

void renderer_draw_filled_cell(int grid_x, int grid_y)
{
    int screen_x = GRID_X_OFFSET + grid_x * GRID_CELL_SIZE;
    int screen_y = GRID_Y_OFFSET + grid_y * GRID_CELL_SIZE;
    
    // Fill the cell with red
    for (int py = 0; py < GRID_CELL_SIZE; py++)
    {
        for (int px = 0; px < GRID_CELL_SIZE; px++)
        {
            dpixel(screen_x + px, screen_y + py, COLOR_TETRIS_RED);
        }
    }
    
    // Draw thin black outline (1 pixel thick)
    // Top edge
    for (int px = 0; px < GRID_CELL_SIZE; px++)
    {
        dpixel(screen_x + px, screen_y, COLOR_BLACK);
    }
    // Bottom edge
    for (int px = 0; px < GRID_CELL_SIZE; px++)
    {
        dpixel(screen_x + px, screen_y + GRID_CELL_SIZE - 1, COLOR_BLACK);
    }
    // Left edge
    for (int py = 0; py < GRID_CELL_SIZE; py++)
    {
        dpixel(screen_x, screen_y + py, COLOR_BLACK);
    }
    // Right edge
    for (int py = 0; py < GRID_CELL_SIZE; py++)
    {
        dpixel(screen_x + GRID_CELL_SIZE - 1, screen_y + py, COLOR_BLACK);
    }
}

void renderer_redraw_all(void)
{
    dclear(COLOR_BACKGROUND);
    grid_draw();
    grid_draw_placed_blocks();
    grid_draw_score();
    tetris_blocks_draw();
    dupdate();
}
