#include <gint/display.h>
#include <gint/rtc.h>
#include <gint/clock.h>
#include <time.h>
#include "tetris_blocks.h"
#include "grid.h"
#include "renderer.h"

// pieces are in 4x4 matrices where 1 = block, 0 = empty
// 7 base pieces × 4 rotations each = 28 total pieces
static const int tetris_pieces[TETRIS_PIECES][4][4] = {
    // I-piece (line) - 4 rotations
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0}
    },
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0}
    },
    
    // O-piece (square) - 4 rotations (all identical)
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    },
    
    // T-piece - 4 rotations
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 1, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0}
    },
    
    // S-piece - 4 rotations
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {1, 1, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0}
    },
    
    // Z-piece - 4 rotations
    {
        {0, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 1, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {1, 0, 0, 0}
    },
    
    // J-piece - 4 rotations
    {
        {0, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 1, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 0, 0}
    },
    
    // L-piece - 4 rotations
    {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {1, 1, 1, 0},
        {1, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0}
    },
    
    // 2x3 piece (6 blocks in 2x3 rectangle)
    {
        {0, 0, 0, 0},
        {1, 1, 0, 0},
        {1, 1, 0, 0},
        {1, 1, 0, 0}
    },
    
    // 3x2 piece (6 blocks in 3x2 rectangle)
    {
        {0, 0, 0, 0},
        {1, 1, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },
    
    // 3x3 piece (9 blocks in 3x3 square) - rare
    {
        {0, 0, 0, 0},
        {1, 1, 1, 0},
        {1, 1, 1, 0},
        {1, 1, 1, 0}
    },
    
    // L pieces
    {
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 1},
        {0, 0, 0, 0}
    },
    {
        {1, 1, 1, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 1, 1, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 0}
    },
    
    // Corner L pieces
    {
        {1, 1, 0, 0},
        {1, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    {
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    {
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    
    // 1x1 block
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    
    // 2x1 block
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    
    // 1x2 block
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0}
    },
    
    // 3x1 block
    {
        {0, 0, 0, 0},
        {0, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    
    // 1x3 block
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0}
    }
};

// Piece difficulty mapping
// 44 pieces: I-piece (0-3): easy, O-piece (4-7): easy, T-piece (8-11): hard, S-piece (12-15): hard, Z-piece (16-19): hard, J-piece (20-23): medium, L-piece (24-27): medium, 2x3 (28): medium, 3x2 (29): medium, 3x3 (30): rare, L variations (31-34): medium, corner L (35-38): medium, small blocks (39-43): rare
static const piece_difficulty_t piece_difficulties[TETRIS_PIECES] = {
    // I-piece rotations (0-3)
    PIECE_EASY, PIECE_EASY, PIECE_EASY, PIECE_EASY,
    // O-piece rotations (4-7)
    PIECE_EASY, PIECE_EASY, PIECE_EASY, PIECE_EASY,
    // T-piece rotations (8-11)
    PIECE_HARD, PIECE_HARD, PIECE_HARD, PIECE_HARD,
    // S-piece rotations (12-15)
    PIECE_HARD, PIECE_HARD, PIECE_HARD, PIECE_HARD,
    // Z-piece rotations (16-19)
    PIECE_HARD, PIECE_HARD, PIECE_HARD, PIECE_HARD,
    // J-piece rotations (20-23)
    PIECE_MEDIUM, PIECE_MEDIUM, PIECE_MEDIUM, PIECE_MEDIUM,
    // L-piece rotations (24-27)
    PIECE_MEDIUM, PIECE_MEDIUM, PIECE_MEDIUM, PIECE_MEDIUM,
    // 2x3 piece (28)
    PIECE_MEDIUM,
    // 3x2 piece (29)
    PIECE_MEDIUM,
    // 3x3 piece (30) - rare
    PIECE_RARE,
    // L piece variations (31-34)
    PIECE_MEDIUM, PIECE_MEDIUM, PIECE_MEDIUM, PIECE_MEDIUM,
    // Corner L piece variations (35-38)
    PIECE_MEDIUM, PIECE_MEDIUM, PIECE_MEDIUM, PIECE_MEDIUM,
    // Small blocks for line breaking (39-43)
    PIECE_RARE,  // 1x1 block
    PIECE_RARE,  // 2x1 block  
    PIECE_RARE,  // 1x2 block
    PIECE_RARE,  // 3x1 block
    PIECE_RARE   // 1x3 block
};

int tetris_piece_cell(int piece_type, int row, int col)
{
    if (piece_type < 0 || piece_type >= TETRIS_PIECES) return 0;
    if (row < 0 || row >= 4 || col < 0 || col >= 4) return 0;
    return tetris_pieces[piece_type][row][col];
}


static int random_seed = 0;

// Selection state
static int selected_block = 0;  // First block is selected by default

// Store the generated pieces
static int stored_pieces[3] = {-1, -1, -1};  // -1 means not generated yet
static uint16_t stored_piece_colors[3] = {0, 0, 0};

static int get_random(void);

static const uint16_t PIECE_PALETTE[7] = {
    0xF800, // red
    0xFD20, // orange
    0xFEA0, // gold
    0x07E0, // green
    0x2F1F, // light blue
    0x22DF, // blue
    0xBA3F, // purple
};

static uint16_t random_palette_color(void)
{
    int idx = get_random() % 7;
    return PIECE_PALETTE[idx];
}

static int get_random(void)
{
    // Use clock for randomness
    uint32_t time = clock();
    random_seed = (random_seed * 1103515245 + 12345) ^ time;
    return (random_seed >> 16) & 0x7FFF;
}

// Check if a small block would perfectly fit to break a line
static int small_block_would_break_line(int piece_type)
{
    // Only check small blocks (39-43)
    if (piece_type < 39 || piece_type > 43) return 0;
    
    // Check all possible positions on the grid
    for (int y = 0; y < GRID_SIZE; y++)
    {
        for (int x = 0; x < GRID_SIZE; x++)
        {
            // Check if this piece can be placed here and would clear lines
            if (grid_can_place(piece_type, x, y) && grid_would_clear_lines(piece_type, x, y))
            {
                return 1; // This small block would break a line
            }
        }
    }
    
    return 0; // No line breaking opportunity found
}

void tetris_blocks_init(void)
{
    // Initialize random seed
    uint32_t time = clock();
    random_seed = time;
    
    // Reset selection to first block
    selected_block = 0;
    
    // Generate the three pieces with weighted spawning and placeability validation (js kill me)
    tetris_generate_valid_pieces();
}

int tetris_blocks_get_selection(void)
{
    return selected_block;
}

void tetris_blocks_set_selection(int selection)
{
    if (selection >= 0 && selection < 3)  // we have 3 blocks
    {
        // Prefer requested selection if available, otherwise skip to next available
        int start = selection;
        for (int i = 0; i < 3; i++)
        {
            int idx = (start + i) % 3;
            if (stored_pieces[idx] >= 0)
            {
                selected_block = idx;
                return;
            }
        }
        // No available pieces; keep selection as requested
        selected_block = selection;
    }
}

static void draw_tetris_piece_core(int x, int y, int piece_type, int is_selected, int block_size, int gap)
{
    if (piece_type < 0 || piece_type >= TETRIS_PIECES) return;

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (tetris_pieces[piece_type][row][col])
            {
                int block_x = x + col * (block_size + gap);
                int block_y = y + row * (block_size + gap);

                renderer_draw_beveled_tile(block_x, block_y, block_size, is_selected);
            }
        }
    }
}

void draw_tetris_piece(int x, int y, int piece_type, int is_selected)
{
    // sidebar preview: slightly smaller than grid cells with 1px gap
    draw_tetris_piece_core(x, y, piece_type, is_selected, TETRIS_BLOCK_SIZE, 1);
}

void draw_tetris_piece_sized(int x, int y, int piece_type, int is_selected, int block_size, int gap)
{
    // general-purpose draw
    draw_tetris_piece_core(x, y, piece_type, is_selected, block_size, gap);
}

void tetris_blocks_draw(void)
{
    // Draw the stored pieces
    // Ensure selection points to an available piece if any
    if (selected_block < 0 || selected_block >= 3 || stored_pieces[selected_block] < 0)
    {
        for (int i = 0; i < 3; i++)
        {
            if (stored_pieces[i] >= 0)
            {
                selected_block = i;
                break;
            }
        }
    }
    for (int i = 0; i < 3; i++)
    {
        int piece_type = stored_pieces[i];
        int y_pos = TETRIS_AREA_Y + i * TETRIS_SPACING;
        int is_selected = (i == selected_block);
        if (piece_type < 0)
        {
            // Skip drawing consumed slots
            continue;
        }
        
        // Ensure sidebar uses the assigned piece color for this slot
        uint16_t color = tetris_blocks_get_piece_color_for_slot(i);
        renderer_set_tile_color(color);
        draw_tetris_piece(TETRIS_AREA_X, y_pos, piece_type, is_selected);
    }
}

int tetris_blocks_get_piece_type_for_selection(int selection)
{
    if (selection < 0 || selection >= 3) return -1;
    // If not generated or consumed, return -1
    if (stored_pieces[selection] < 0 || stored_pieces[selection] >= TETRIS_PIECES)
        return -1;
    return stored_pieces[selection];
}


void tetris_blocks_consume_selected(void)
{
    if (selected_block < 0 || selected_block >= 3) return;
    stored_pieces[selected_block] = -1; // mark consumed
    stored_piece_colors[selected_block] = 0;

    // Move selection to next available piece if any
    for (int i = 0; i < 3; i++)
    {
        int idx = (selected_block + i) % 3;
        if (stored_pieces[idx] >= 0)
        {
            selected_block = idx;
            return;
        }
    }
    // None left; don't regenerate immediately - wait for piece to be placed
}

void tetris_blocks_regenerate_if_needed(void)
{
    // Check if all pieces are consumed and regenerate if needed
    int remaining = 0;
    for (int i = 0; i < 3; i++)
    {
        if (stored_pieces[i] >= 0) remaining++;
    }
    if (remaining == 0)
    {
        // Generate three new pieces with weighted spawning and placeability validation
        tetris_generate_valid_pieces();
    }
}

void tetris_blocks_get_available_pieces(int pieces[], int *count)
{
    int available_count = 0;
    for (int i = 0; i < 3; i++)
    {
        if (stored_pieces[i] >= 0)
        {
            pieces[available_count] = stored_pieces[i];
            available_count++;
        }
    }
    *count = available_count;
}

void tetris_blocks_restore_piece(int piece_type)
{
    if (piece_type < 0 || piece_type >= TETRIS_PIECES) return;
    
    // Find the first empty slot to restore the piece
    for (int i = 0; i < 3; i++)
    {
        if (stored_pieces[i] < 0) // Empty slot found
        {
            stored_pieces[i] = piece_type;
            stored_piece_colors[i] = random_palette_color();
            // Set this as the selected piece
            selected_block = i;
            return;
        }
    }
    
    // If no empty slot found, replace the current selection
    if (selected_block >= 0 && selected_block < 3)
    {
        stored_pieces[selected_block] = piece_type;
        stored_piece_colors[selected_block] = random_palette_color();
    }
}

void tetris_blocks_restore_piece_with_color(int piece_type, uint16_t color)
{
    if (piece_type < 0 || piece_type >= TETRIS_PIECES) return;
    // find first empty slot
    for (int i = 0; i < 3; i++)
    {
        if (stored_pieces[i] < 0)
        {
            stored_pieces[i] = piece_type;
            stored_piece_colors[i] = color;
            selected_block = i;
            return;
        }
    }
    // (fallback): overwrite selection
    if (selected_block >= 0 && selected_block < 3)
    {
        stored_pieces[selected_block] = piece_type;
        stored_piece_colors[selected_block] = color;
    }
}

piece_difficulty_t tetris_get_piece_difficulty(int piece_type)
{
    if (piece_type < 0 || piece_type >= TETRIS_PIECES) return PIECE_EASY;
    return piece_difficulties[piece_type];
}

int tetris_piece_is_placeable(int piece_type)
{
    if (piece_type < 0 || piece_type >= TETRIS_PIECES) return 0;
    
    // Check if piece can be placed anywhere on the current grid
    for (int gy = 0; gy < GRID_SIZE; gy++)
    {
        for (int gx = 0; gx < GRID_SIZE; gx++)
        {
            if (grid_can_place(piece_type, gx, gy))
            {
                return 1; // Piece can be placed at least somewhere
            }
        }
    }
    return 0; // Piece cannot be placed anywhere
}

int tetris_generate_weighted_piece(void)
{
    // Calculate total weight for all pieces
    int total_weight = 0;
    for (int i = 0; i < TETRIS_PIECES; i++)
    {
        piece_difficulty_t difficulty = piece_difficulties[i];
        switch (difficulty)
        {
            case PIECE_EASY:   total_weight += EASY_WEIGHT; break;
            case PIECE_MEDIUM: total_weight += MEDIUM_WEIGHT; break;
            case PIECE_HARD:   total_weight += HARD_WEIGHT; break;
            case PIECE_RARE:   total_weight += RARE_WEIGHT; break;
        }
    }
    
    // Generate random number in range [0, total_weight)
    int random_value = get_random() % total_weight;
    
    // Find which piece this random value corresponds to
    int current_weight = 0;
    for (int i = 0; i < TETRIS_PIECES; i++)
    {
        piece_difficulty_t difficulty = piece_difficulties[i];
        int piece_weight = 0;
        switch (difficulty)
        {
            case PIECE_EASY:   piece_weight = EASY_WEIGHT; break;
            case PIECE_MEDIUM: piece_weight = MEDIUM_WEIGHT; break;
            case PIECE_HARD:   piece_weight = HARD_WEIGHT; break;
            case PIECE_RARE:   piece_weight = RARE_WEIGHT; break;
        }
        
        current_weight += piece_weight;
        if (random_value < current_weight)
        {
            return i; // This is the selected piece
        }
    }
    
    // Fallback (should never reach here)
    return 0;
}

void tetris_generate_valid_pieces(void)
{
    int spawned_types[TETRIS_PIECES] = {0}; // Track which types we've spawned
    int attempts = 0;
    const int max_attempts = 100; // Prevent infinite loops
    const int small_block_chance = 8; // 8% chance for small blocks (1-100)
    
    // Clear current pieces
    for (int i = 0; i < 3; i++)
    {
        stored_pieces[i] = -1;
        stored_piece_colors[i] = 0;
    }
    
    // Generate 3 valid pieces
    for (int slot = 0; slot < 3; slot++)
    {
        int piece_type = -1;
        attempts = 0;
        
        // Check if we should try to spawn a small block for line breaking
        int try_small_block = (get_random() % 100) < small_block_chance;
        
        // Keep trying until we find a valid piece
        while (attempts < max_attempts)
        {
            int candidate;
            
            // If we should try small blocks and haven't tried them yet
            if (try_small_block && attempts < 5)
            {
                // Try small blocks (39-43) that would break lines
                for (int small_piece = 39; small_piece <= 43; small_piece++)
                {
                    if (!spawned_types[small_piece] && 
                        tetris_piece_is_placeable(small_piece) &&
                        small_block_would_break_line(small_piece))
                    {
                        piece_type = small_piece;
                        spawned_types[small_piece] = 1;
                        break;
                    }
                }
                if (piece_type != -1) break;
            }
            
            // Generate a weighted random piece
            candidate = tetris_generate_weighted_piece();
            
            // Check if we already spawned this type
            if (spawned_types[candidate])
            {
                attempts++;
                continue;
            }
            
            // Check if this piece is placeable
            if (tetris_piece_is_placeable(candidate))
            {
                piece_type = candidate;
                spawned_types[candidate] = 1; // Mark as spawned
                break;
            }
            
            attempts++;
        }
        
        // If we couldn't find a valid piece, use the first available one
        if (piece_type == -1)
        {
            for (int i = 0; i < TETRIS_PIECES; i++)
            {
                if (!spawned_types[i] && tetris_piece_is_placeable(i))
                {
                    piece_type = i;
                    spawned_types[i] = 1;
                    break;
                }
            }
        }
        
        // If still no valid piece, use any piece (fallback)
        if (piece_type == -1)
        {
            for (int i = 0; i < TETRIS_PIECES; i++)
            {
                if (!spawned_types[i])
                {
                    piece_type = i;
                    spawned_types[i] = 1;
                    break;
                }
            }
        }
        
        stored_pieces[slot] = piece_type;
        stored_piece_colors[slot] = random_palette_color();
    }
    
    // Reset selection to first piece
    selected_block = 0;
}

uint16_t tetris_blocks_get_piece_color_for_slot(int slot)
{
    if (slot < 0 || slot >= 3) return COLOR_TETRIS_RED;
    if (stored_pieces[slot] < 0) return COLOR_TETRIS_RED;
    uint16_t c = stored_piece_colors[slot];
    return c ? c : COLOR_TETRIS_RED;
}

uint16_t tetris_blocks_get_color_for_piece_type(int piece_type)
{
    // Find the piece in the sidebar and return its assigned color
    for (int i = 0; i < 3; i++)
    {
        if (stored_pieces[i] == piece_type)
        {
            uint16_t c = stored_piece_colors[i];
            return c ? c : COLOR_TETRIS_RED;
        }
    }
    // Default
    return COLOR_TETRIS_RED;
}
