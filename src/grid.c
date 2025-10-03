#include <gint/display.h>
#include "grid.h"
#include "tetris_blocks.h"
#include "score.h"
#include "font.h"
#include "renderer.h"

// Screen dims
#define SCREEN_WIDTH 396
#define SCREEN_HEIGHT 224

// color for particles
#define COLOR_TETRIS_RED 0xF800

// Grid state
static placed_block_t placed_blocks[MAX_PLACED_BLOCKS];
static int num_placed_blocks = 0;
static int active_block_index = -1;  // -1 means no active block

// Persistent occupancy of the 8x8 grid locked cells
static int grid_occupied[GRID_SIZE][GRID_SIZE];

// Internal render state
static int is_animating = 0; // 1 while performing line-clear animation

// simple particle system
#define MAX_PARTICLES 256
typedef struct {
	int active;
	int x;
	int y;
	int vx;
	int vy;
	int life;
} particle_t;

static particle_t particles[MAX_PARTICLES];

// PRNG
static unsigned int rng_state = 123456789u;
static unsigned int prng_next(void)
{
	// xorshift32
	unsigned int x = rng_state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	rng_state = x;
	return x;
}

static int rand_range(int min_inclusive, int max_inclusive)
{
	unsigned int r = prng_next();
	int span = max_inclusive - min_inclusive + 1;
	if (span <= 0) return min_inclusive;
	return min_inclusive + (int)(r % (unsigned int)span);
}

static void spawn_cell_explosion(int grid_x, int grid_y)
{
	int cell_x = GRID_X_OFFSET + grid_x * GRID_CELL_SIZE;
	int cell_y = GRID_Y_OFFSET + grid_y * GRID_CELL_SIZE;
	int cx = cell_x + GRID_CELL_SIZE / 2;
	int cy = cell_y + GRID_CELL_SIZE / 2;

	// Spawn a handful of particles
	for (int i = 0; i < 6; i++)
	{
		// Find a free slot
		for (int p = 0; p < MAX_PARTICLES; p++)
		{
			if (!particles[p].active)
			{
				particles[p].active = 1;
				particles[p].x = cx;
				particles[p].y = cy;
				particles[p].vx = rand_range(-2, 2);
				particles[p].vy = rand_range(-3, -1);
				particles[p].life = rand_range(6, 12);
				break;
			}
		}
	}
}

static void update_and_draw_particles(void)
{
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if (!particles[i].active) continue;
		// Update
		particles[i].x += particles[i].vx;
		particles[i].y += particles[i].vy;
		// gravity
		particles[i].vy += 1;
		particles[i].life -= 1;
		if (particles[i].life <= 0) {
			particles[i].active = 0;
			continue;
		}
		// Draw as a 2x2 square for a bigger particle
		for (int py = 0; py < 2; py++)
		{
			for (int px = 0; px < 2; px++)
			{
				dpixel(particles[i].x + px, particles[i].y + py, COLOR_TETRIS_RED);
			}
		}
	}
}


// draw a single filled cell at grid coords with outline
static void draw_filled_cell(int grid_x, int grid_y)
{
	renderer_draw_filled_cell(grid_x, grid_y);
}

// stamp a piece's filled cells into occupancy grid
static void stamp_piece_into_occupancy(int piece_type, int gx, int gy)
{
	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			if (!tetris_piece_cell(piece_type, row, col)) continue;
			int cx = gx + col;
			int cy = gy + row;
			if (cx >= 0 && cy >= 0 && cx < GRID_SIZE && cy < GRID_SIZE)
			{
				grid_occupied[cy][cx] = 1;
			}
		}
	}
}

// check if a piece at (gx, gy) would overlap any occupied cell
static int piece_overlaps_occupancy(int piece_type, int gx, int gy)
{
	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			if (!tetris_piece_cell(piece_type, row, col)) continue;
			int cx = gx + col;
			int cy = gy + row;
			if (cx < 0 || cy < 0 || cx >= GRID_SIZE || cy >= GRID_SIZE) continue;
			if (grid_occupied[cy][cx]) return 1;
		}
	}
	return 0;
}

// after stamping a piece, clear full rows or column
static void clear_full_lines(void)
{
	int full_rows[GRID_SIZE] = {0};
	int full_cols[GRID_SIZE] = {0};
	int lines_cleared = 0;

	// detect full rows
	for (int y = 0; y < GRID_SIZE; y++)
	{
		int all_filled = 1;
		for (int x = 0; x < GRID_SIZE; x++)
		{
			if (!grid_occupied[y][x]) { all_filled = 0; break; }
		}
		full_rows[y] = all_filled;
		if (all_filled) lines_cleared++;
	}

	// detect full columns
	for (int x = 0; x < GRID_SIZE; x++)
	{
		int all_filled = 1;
		for (int y = 0; y < GRID_SIZE; y++)
		{
			if (!grid_occupied[y][x]) { all_filled = 0; break; }
		}
		full_cols[x] = all_filled;
		if (all_filled) lines_cleared++;
	}


	// If no lines to clear, return early
	if (lines_cleared == 0)
	{
		return;
	}

	// Animate clearing as a sweep: rows left->right, cols top->bottom
	is_animating = 1;
	for (int step = 0; step < GRID_SIZE; step++)
	{
		// For each full row, clear the next cell from left to right
		for (int y = 0; y < GRID_SIZE; y++)
		{
			if (full_rows[y])
			{
				int x = step;
				if (x >= 0 && x < GRID_SIZE)
				{
					if (grid_occupied[y][x])
					{
						grid_occupied[y][x] = 0;
						spawn_cell_explosion(x, y);
					}
				}
			}
		}
		// For each full column, clear the next cell from top to bottom
		for (int x = 0; x < GRID_SIZE; x++)
		{
			if (full_cols[x])
			{
				int y = step;
				if (y >= 0 && y < GRID_SIZE)
				{
					if (grid_occupied[y][x])
					{
						grid_occupied[y][x] = 0;
						spawn_cell_explosion(x, y);
					}
				}
			}
		}

		// Redraw the scene after this step
		dclear(COLOR_BACKGROUND);
		grid_draw();
		grid_draw_placed_blocks();
		grid_draw_score();
		tetris_blocks_draw();
		update_and_draw_particles();
		dupdate();

		// Small delay for visible animation (busy-wait)
		for (volatile int w = 0; w < 120000; w++) { }
	}
	is_animating = 0;

	// award points based on lines cleared
	if (lines_cleared == 1)
		score_add_points(SCORE_LINE_CLEAR_1);
	else if (lines_cleared == 2)
		score_add_points(SCORE_LINE_CLEAR_2);
	else if (lines_cleared == 3)
		score_add_points(SCORE_LINE_CLEAR_3);
	else if (lines_cleared >= 4)
		score_add_points(SCORE_LINE_CLEAR_4_PLUS);
}

void grid_init(void)
{
    // Set the background color
    dclear(COLOR_BACKGROUND);
    
    // Initialize placed blocks array
    for (int i = 0; i < MAX_PLACED_BLOCKS; i++)
    {
        placed_blocks[i].piece_type = BLOCK_TYPE_EMPTY;
        placed_blocks[i].grid_x = 0;
        placed_blocks[i].grid_y = 0;
        placed_blocks[i].is_active = 0;
    }
    num_placed_blocks = 0;
    active_block_index = -1;

	// Clear occupancy grid
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			grid_occupied[y][x] = 0;
		}
	}
	
	// Reset score
	score_init();
}

void grid_draw(void)
{
    // Draw vertical grid lines
    for(int i = 0; i <= GRID_SIZE; i++)
    {
        int x = GRID_X_OFFSET + (i * GRID_CELL_SIZE);
        dline(x, GRID_Y_OFFSET, x, 
              GRID_Y_OFFSET + GRID_SIZE * GRID_CELL_SIZE, 
              COLOR_GRID_LINE);
    }
    
    // Draw horizontal grid lines
    for(int i = 0; i <= GRID_SIZE; i++)
    {
        int y = GRID_Y_OFFSET + (i * GRID_CELL_SIZE);
        dline(GRID_X_OFFSET, y, 
              GRID_X_OFFSET + GRID_SIZE * GRID_CELL_SIZE, y, 
              COLOR_GRID_LINE);
    }
}

void grid_clear(void)
{
    // Clear the grid area (8x8 centered)
    drect(GRID_X_OFFSET, GRID_Y_OFFSET, 
          GRID_SIZE * GRID_CELL_SIZE, 
          GRID_SIZE * GRID_CELL_SIZE, 
          COLOR_BACKGROUND);
}

void grid_place_block(int piece_type, int grid_x, int grid_y)
{
    if (num_placed_blocks >= MAX_PLACED_BLOCKS) return;
    
    // Find an empty slot
    for (int i = 0; i < MAX_PLACED_BLOCKS; i++)
    {
        if (placed_blocks[i].piece_type == BLOCK_TYPE_EMPTY)
        {
            placed_blocks[i].piece_type = piece_type;
            placed_blocks[i].grid_x = grid_x;
            placed_blocks[i].grid_y = grid_y;
            placed_blocks[i].is_active = 1;  // Newly placed block is active
            num_placed_blocks++;
            
            // Set as active block
            active_block_index = i;
            break;
        }
    }
}

void grid_move_active_block(int dx, int dy)
{
    if (active_block_index == -1) return;
    
    int new_x = placed_blocks[active_block_index].grid_x + dx;
    int new_y = placed_blocks[active_block_index].grid_y + dy;
    
    // Check if new position is valid
    if (grid_is_valid_position(placed_blocks[active_block_index].piece_type, new_x, new_y))
    {
        placed_blocks[active_block_index].grid_x = new_x;
        placed_blocks[active_block_index].grid_y = new_y;
    }
}

void grid_finalize_active_block(void)
{
    if (active_block_index == -1) return;
    
    // Award points for piece placement
    score_add_placement();
    
    // Stamp the active piece into occupancy
    stamp_piece_into_occupancy(
		placed_blocks[active_block_index].piece_type,
		placed_blocks[active_block_index].grid_x,
		placed_blocks[active_block_index].grid_y);

    // Clear any full rows/columns
    clear_full_lines();

    // Remove the active block from the temp array
    placed_blocks[active_block_index].piece_type = BLOCK_TYPE_EMPTY;
    placed_blocks[active_block_index].is_active = 0;
    num_placed_blocks--;
    active_block_index = -1;
}

// Removed unused cells_overlap function

int grid_active_overlaps_existing(void)
{
    if (active_block_index == -1) return 0;
    int piece_type = placed_blocks[active_block_index].piece_type;
    int gx = placed_blocks[active_block_index].grid_x;
    int gy = placed_blocks[active_block_index].grid_y;
    return piece_overlaps_occupancy(piece_type, gx, gy);
}

int grid_would_overlap(int piece_type, int grid_x, int grid_y)
{
    // Check overlap against persistent occupancy
    return piece_overlaps_occupancy(piece_type, grid_x, grid_y);
}

int grid_can_place(int piece_type, int grid_x, int grid_y)
{
    if (!grid_is_valid_position(piece_type, grid_x, grid_y)) return 0;
    if (grid_would_overlap(piece_type, grid_x, grid_y)) return 0;
    return 1;
}

int grid_find_first_fit(int piece_type, int *out_x, int *out_y)
{
    if (piece_type < 0) return 0;
    for (int gy = 0; gy < GRID_SIZE; gy++)
    {
        for (int gx = 0; gx < GRID_SIZE; gx++)
        {
            if (grid_can_place(piece_type, gx, gy))
            {
                if (out_x) *out_x = gx;
                if (out_y) *out_y = gy;
                return 1;
            }
        }
    }
    return 0;
}

// Simulate placing a piece and check if it would clear any rows/columns
static int simulate_placement_with_clearing(int piece_type, int gx, int gy)
{
    // Create a temporary copy of the occupancy grid
    int temp_occupied[GRID_SIZE][GRID_SIZE];
    for (int y = 0; y < GRID_SIZE; y++)
    {
        for (int x = 0; x < GRID_SIZE; x++)
        {
            temp_occupied[y][x] = grid_occupied[y][x];
        }
    }
    
    // Place the piece in the temporary grid
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (!tetris_piece_cell(piece_type, row, col)) continue;
            int cx = gx + col;
            int cy = gy + row;
            if (cx >= 0 && cy >= 0 && cx < GRID_SIZE && cy < GRID_SIZE)
            {
                temp_occupied[cy][cx] = 1;
            }
        }
    }
    
    // Check for full rows and columns in the temporary grid
    int full_rows[GRID_SIZE] = {0};
    int full_cols[GRID_SIZE] = {0};
    
    // Detect full rows
    for (int y = 0; y < GRID_SIZE; y++)
    {
        int all_filled = 1;
        for (int x = 0; x < GRID_SIZE; x++)
        {
            if (!temp_occupied[y][x]) { all_filled = 0; break; }
        }
        full_rows[y] = all_filled;
    }
    
    // Detect full columns
    for (int x = 0; x < GRID_SIZE; x++)
    {
        int all_filled = 1;
        for (int y = 0; y < GRID_SIZE; y++)
        {
            if (!temp_occupied[y][x]) { all_filled = 0; break; }
        }
        full_cols[x] = all_filled;
    }
    
    // Clear marked rows and columns in the temporary grid
    for (int y = 0; y < GRID_SIZE; y++)
    {
        for (int x = 0; x < GRID_SIZE; x++)
        {
            if (full_rows[y] || full_cols[x])
            {
                temp_occupied[y][x] = 0;
            }
        }
    }
    
    // check if any piece can fit in the cleared grid
    for (int i = 0; i < TETRIS_PIECES; i++) // Check all piece types
    {
        for (int gy = 0; gy < GRID_SIZE; gy++)
        {
            for (int gx = 0; gx < GRID_SIZE; gx++)
            {
                // check if this piece can fit at this position in the cleared grid
                int can_fit = 1;
                
                // check bounds
                for (int row = 0; row < 4; row++)
                {
                    for (int col = 0; col < 4; col++)
                    {
                        if (tetris_piece_cell(i, row, col))
                        {
                            int cell_x = gx + col;
                            int cell_y = gy + row;
                            if (cell_x < 0 || cell_y < 0 || cell_x >= GRID_SIZE || cell_y >= GRID_SIZE)
                            {
                                can_fit = 0;
                                break;
                            }
                        }
                    }
                    if (!can_fit) break;
                }
                
                if (!can_fit) continue;
                
                // Check for overlaps with cleared grid
                for (int row = 0; row < 4; row++)
                {
                    for (int col = 0; col < 4; col++)
                    {
                        if (tetris_piece_cell(i, row, col))
                        {
                            int cx = gx + col;
                            int cy = gy + row;
                            if (temp_occupied[cy][cx])
                            {
                                can_fit = 0;
                                break;
                            }
                        }
                    }
                    if (!can_fit) break;
                }
                
                if (can_fit)
                {
                    return 1; // At least one piece can fit after clearing
                }
            }
        }
    }
    
    return 0; // No pieces can fit even after clearing
}

int grid_can_any_piece_fit(int available_pieces[], int num_pieces)
{
    // First check if any piece can fit without clearing
    for (int i = 0; i < num_pieces; i++)
    {
        int piece_type = available_pieces[i];
        if (piece_type < 0) continue; // Skip invalid pieces
        
        // Try all possible positions for this piece
        for (int gy = 0; gy < GRID_SIZE; gy++)
        {
            for (int gx = 0; gx < GRID_SIZE; gx++)
            {
                if (grid_can_place(piece_type, gx, gy))
                {
                    return 1; // At least one piece can fit without clearing
                }
            }
        }
    }
    
    // If no piece can fit without clearing, check if any piece can fit after clearing
    for (int i = 0; i < num_pieces; i++)
    {
        int piece_type = available_pieces[i];
        if (piece_type < 0) continue; // Skip invalid pieces
        
        // Try all possible positions for this piece and check if clearing would help
        for (int gy = 0; gy < GRID_SIZE; gy++)
        {
            for (int gx = 0; gx < GRID_SIZE; gx++)
            {
                // Check if piece can be placed (even if it overlaps)
                if (grid_is_valid_position(piece_type, gx, gy))
                {
                    if (simulate_placement_with_clearing(piece_type, gx, gy))
                    {
                        return 1; // This piece can be placed and would clear space for more pieces
                    }
                }
            }
        }
    }
    
    return 0; // No pieces can fit anywhere, even with clearing
}

// Font rendering is now handled in font.c

int grid_cancel_active_block(void)
{
    if (active_block_index == -1) return -1; // No active block to cancel
    
    // Get the piece type before removing it
    int piece_type = placed_blocks[active_block_index].piece_type;
    
    // Remove the active block from the temp array
    placed_blocks[active_block_index].piece_type = BLOCK_TYPE_EMPTY;
    placed_blocks[active_block_index].is_active = 0;
    num_placed_blocks--;
    active_block_index = -1;
    
    return piece_type; // Return the piece type so it can be restored to sidebar
}

void grid_set_active_block(int block_index)
{
    if (block_index >= 0 && block_index < MAX_PLACED_BLOCKS && 
        placed_blocks[block_index].piece_type != BLOCK_TYPE_EMPTY)
    {
        // Deactivate current active block
        if (active_block_index != -1)
        {
            placed_blocks[active_block_index].is_active = 0;
        }
        
        // Activate new block
        active_block_index = block_index;
        placed_blocks[active_block_index].is_active = 1;
    }
}

int grid_get_active_block(void)
{
    return active_block_index;
}

int grid_is_valid_position(int piece_type, int grid_x, int grid_y)
{
    // Validate the entire 4x4 piece footprint against the grid
    if (piece_type < 0) return 0;

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (tetris_piece_cell(piece_type, row, col))
            {
                int cell_x = grid_x + col;
                int cell_y = grid_y + row;
                if (cell_x < 0 || cell_y < 0 || cell_x >= GRID_SIZE || cell_y >= GRID_SIZE)
                    return 0;
            }
        }
    }
    return 1;
}

void grid_draw_placed_blocks(void)
{
    // First draw all locked cells from occupancy (non-active color)
    for (int y = 0; y < GRID_SIZE; y++)
    {
        for (int x = 0; x < GRID_SIZE; x++)
        {
            if (grid_occupied[y][x])
            {
                draw_filled_cell(x, y);
            }
        }
    }

    // Then draw the active block if any (selected color handled by draw function)
	if (!is_animating && active_block_index != -1 && placed_blocks[active_block_index].piece_type != BLOCK_TYPE_EMPTY)
    {
        int screen_x = GRID_X_OFFSET + placed_blocks[active_block_index].grid_x * GRID_CELL_SIZE;
        int screen_y = GRID_Y_OFFSET + placed_blocks[active_block_index].grid_y * GRID_CELL_SIZE;
        draw_tetris_piece_sized(screen_x, screen_y,
            placed_blocks[active_block_index].piece_type, 1, GRID_CELL_SIZE, 0);
    }
}

int grid_get_score(void)
{
    return score_get_current();
}

void grid_draw_score(void)
{
    score_draw();
}
