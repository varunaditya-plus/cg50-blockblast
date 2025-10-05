#ifndef RENDERER_H
#define RENDERER_H

// Rendering functions
void renderer_draw_game_over(void);
void renderer_draw_filled_cell(int grid_x, int grid_y);
void renderer_redraw_all(void);
void renderer_draw_beveled_tile(int x, int y, int size, int is_selected);
void renderer_draw_footer(void);
// Set base color used for tiles (RGB565)
void renderer_set_tile_color(uint16_t color);
// Blend two RGB565 colors; t in [0..255]
uint16_t renderer_blend565(uint16_t a, uint16_t b, int t);

#endif // RENDERER_H
