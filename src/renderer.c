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
#define COLOR_WHITE 0xFFFF       // White in RGB565

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

static inline uint16_t blend565(uint16_t a, uint16_t b, int t /*0..255*/)
{
    // Linear blend a*(255-t)/255 + b*t/255 in RGB565
    int ar = (a >> 11) & 0x1F, ag = (a >> 5) & 0x3F, ab = a & 0x1F;
    int br = (b >> 11) & 0x1F, bg = (b >> 5) & 0x3F, bb = b & 0x1F;
    int rr = (ar * (255 - t) + br * t) / 255;
    int rg = (ag * (255 - t) + bg * t) / 255;
    int rb = (ab * (255 - t) + bb * t) / 255;
    return (uint16_t)((rr << 11) | (rg << 5) | rb);
}

uint16_t renderer_blend565(uint16_t a, uint16_t b, int t)
{
    return blend565(a, b, t);
}

static uint16_t g_tile_base_color = COLOR_TETRIS_RED;

void renderer_set_tile_color(uint16_t color)
{
    g_tile_base_color = color;
}

static void draw_bevel_tile_internal(int x, int y, int size, int is_selected)
{
    // Base color with darker shading
    const uint16_t base = g_tile_base_color; // configurable base color
    const uint16_t deep = blend565(base, COLOR_BLACK, 180); // darker red

    for (int py = 0; py < size; py++)
    {
        for (int px = 0; px < size; px++)
        {
            // radialish gradient weight
            int dx = (px - size / 2);
            int dy = (py - size / 2);
            int d2 = dx*dx + dy*dy;
            int maxd2 = (size*size)/2;
            if (maxd2 <= 0) maxd2 = 1;
            int t = d2 * 255 / maxd2; if (t > 255) t = 255;
            uint16_t col = blend565(base, deep, t/2);
            dpixel(x + px, y + py, col);
        }
    }

    // Bevel: light top-left, dark bottom-right
    int rim = size / 6; if (rim < 2) rim = 2; if (rim > 4) rim = 4;
    for (int i = 0; i < rim; i++)
    {
        uint16_t edgeLight = blend565(base, COLOR_WHITE, 160 - i*40);
        uint16_t edgeDark = blend565(base, deep, 200);
        // top
        for (int px = i; px < size - i; px++) dpixel(x + px, y + i, edgeLight);
        // left
        for (int py = i; py < size - i; py++) dpixel(x + i, y + py, edgeLight);
        // bottom
        for (int px = i; px < size - i; px++) dpixel(x + px, y + size - 1 - i, edgeDark);
        // right
        for (int py = i; py < size - i; py++) dpixel(x + size - 1 - i, y + py, edgeDark);
    }


    // Thin outer border for definition
    for (int px = 0; px < size; px++) { dpixel(x + px, y, COLOR_BLACK); dpixel(x + px, y + size - 1, COLOR_BLACK); }
    for (int py = 0; py < size; py++) { dpixel(x, y + py, COLOR_BLACK); dpixel(x + size - 1, y + py, COLOR_BLACK); }

    // selection outline (on top of black border)
    if (is_selected)
    {
        for (int px = 0; px < size; px++) { dpixel(x + px, y, COLOR_WHITE); dpixel(x + px, y + size - 1, COLOR_WHITE); }
        for (int py = 0; py < size; py++) { dpixel(x, y + py, COLOR_WHITE); dpixel(x + size - 1, y + py, COLOR_WHITE); }
    }
}

void renderer_draw_beveled_tile(int x, int y, int size, int is_selected)
{
    draw_bevel_tile_internal(x, y, size, is_selected);
}

void renderer_draw_filled_cell(int grid_x, int grid_y)
{
    int screen_x = GRID_X_OFFSET + grid_x * GRID_CELL_SIZE;
    int screen_y = GRID_Y_OFFSET + grid_y * GRID_CELL_SIZE;
    draw_bevel_tile_internal(screen_x, screen_y, GRID_CELL_SIZE, 0);
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
