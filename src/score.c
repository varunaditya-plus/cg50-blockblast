#include <gint/display.h>
#include "score.h"
#include "font.h"
#include "grid.h"

// Grid colors (RGB565 format for CG-50)
#define COLOR_BACKGROUND 0x3270  // #364C87 converted to RGB565

// Grid position (centered on screen)
#define GRID_X_OFFSET 118  // (396 - 160) / 2
#define GRID_Y_OFFSET 32   // (224 - 160) / 2
#define GRID_SIZE 8
#define GRID_CELL_SIZE 20  // 20x20 pixel cells

// Score tracking
static int current_score = 0;

void score_init(void)
{
    current_score = 0;
}

int score_get_current(void)
{
    return current_score;
}

void score_add_placement(void)
{
    current_score += SCORE_PIECE_PLACEMENT;
}

void score_add_points(int points)
{
    current_score += points;
}

void score_clear_lines(void)
{
    // will be called by grid when lines are cleared
    // ts is a placeholder cause i plan to do sm here later
}

void score_draw(void)
{
    // Position score on the right side of the grid
    int score_x = GRID_X_OFFSET + GRID_SIZE * GRID_CELL_SIZE + 20; // Right of grid
    int score_y = GRID_Y_OFFSET + 20; // Below top of grid
    
    // Draw "SCORE:" label
    font_draw_text(score_x, score_y, "SCORE:");
    
    // Convert score to string and draw it
    char score_str[20];
    int score = current_score;
    
    // Handle zero case
    if (score == 0)
    {
        score_str[0] = '0';
        score_str[1] = '\0';
    }
    else
    {
        // Convert number to string (reverse order)
        char temp[20];
        int temp_len = 0;
        while (score > 0)
        {
            temp[temp_len++] = '0' + (score % 10);
            score /= 10;
        }
        
        // Reverse to get correct order
        for (int i = 0; i < temp_len; i++)
        {
            score_str[i] = temp[temp_len - 1 - i];
        }
        score_str[temp_len] = '\0';
    }
    
    // Draw the score number
    font_draw_text(score_x, score_y + 12, score_str);
}
