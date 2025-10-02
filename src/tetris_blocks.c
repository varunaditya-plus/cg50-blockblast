#include <gint/display.h>
#include <gint/rtc.h>
#include <gint/clock.h>
#include <time.h>
#include "tetris_blocks.h"
#include "grid.h"

// pieces are in 4x4 matrices where 1 = block, 0 = empty
static const int tetris_pieces[TETRIS_PIECES][4][4] = {
    // I-piece (line)
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // O-piece (square)
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    },
    // T-piece
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },
    // S-piece
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0}
    },
    // Z-piece
    {
        {0, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    },
    // J-piece
    {
        {0, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },
    // L-piece
    {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    }
};

// Piece difficulty mapping
// 0=I (easy), 1=O (easy), 2=T (hard), 3=S (hard), 4=Z (hard), 5=J (medium), 6=L (medium)
static const piece_difficulty_t piece_difficulties[TETRIS_PIECES] = {
    PIECE_EASY,   // I-piece (straight line)
    PIECE_EASY,   // O-piece (square/block)
    PIECE_HARD,   // T-piece
    PIECE_HARD,   // S-piece (zigzag)
    PIECE_HARD,   // Z-piece (zigzag)
    PIECE_MEDIUM, // J-piece (L shape)
    PIECE_MEDIUM  // L-piece (L shape)
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

static int get_random(void)
{
    // Use clock for randomness
    uint32_t time = clock();
    random_seed = (random_seed * 1103515245 + 12345) ^ time;
    return (random_seed >> 16) & 0x7FFF;
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

                if (is_selected)
                {
                    // Draw white outline for selected pieces
                    for (int py = 0; py < block_size; py++)
                    {
                        for (int px = 0; px < block_size; px++)
                        {
                            dpixel(block_x + px, block_y + py, COLOR_TETRIS_RED);
                        }
                    }
                    // Draw  white outline
                    for (int outline = 0; outline < 3; outline++)
                    {
                        // Top edge
                        for (int px = 0; px < block_size; px++)
                        {
                            if (px + outline < block_size)
                                dpixel(block_x + px, block_y + outline, COLOR_TETRIS_WHITE);
                        }
                        // Bottom edge
                        for (int px = 0; px < block_size; px++)
                        {
                            if (px + outline < block_size)
                                dpixel(block_x + px, block_y + block_size - 1 - outline, COLOR_TETRIS_WHITE);
                        }
                        // Left edge
                        for (int py = 0; py < block_size; py++)
                        {
                            if (py + outline < block_size)
                                dpixel(block_x + outline, block_y + py, COLOR_TETRIS_WHITE);
                        }
                        // Right edge
                        for (int py = 0; py < block_size; py++)
                        {
                            if (py + outline < block_size)
                                dpixel(block_x + block_size - 1 - outline, block_y + py, COLOR_TETRIS_WHITE);
                        }
                    }
                }
                else
                {
                    // Draw solid red for non-selected pieces
                    for (int py = 0; py < block_size; py++)
                    {
                        for (int px = 0; px < block_size; px++)
                        {
                            dpixel(block_x + px, block_y + py, COLOR_TETRIS_RED);
                        }
                    }
                }
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
            // Set this as the selected piece
            selected_block = i;
            return;
        }
    }
    
    // If no empty slot found, replace the current selection
    if (selected_block >= 0 && selected_block < 3)
    {
        stored_pieces[selected_block] = piece_type;
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
    
    // Clear current pieces
    for (int i = 0; i < 3; i++)
    {
        stored_pieces[i] = -1;
    }
    
    // Generate 3 valid pieces
    for (int slot = 0; slot < 3; slot++)
    {
        int piece_type = -1;
        attempts = 0;
        
        // Keep trying until we find a valid piece
        while (attempts < max_attempts)
        {
            // Generate a weighted random piece
            int candidate = tetris_generate_weighted_piece();
            
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
    }
    
    // Reset selection to first piece
    selected_block = 0;
}
