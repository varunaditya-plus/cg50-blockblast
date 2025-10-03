#ifndef RENDERER_H
#define RENDERER_H

// Rendering functions
void renderer_draw_game_over(void);
void renderer_draw_filled_cell(int grid_x, int grid_y);
void renderer_redraw_all(void);
void renderer_draw_beveled_tile(int x, int y, int size, int is_selected);

#endif // RENDERER_H
