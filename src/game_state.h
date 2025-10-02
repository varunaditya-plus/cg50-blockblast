#ifndef GAME_STATE_H
#define GAME_STATE_H

// Game state management
void game_state_init(void);
void game_state_reset(void);
int game_state_is_over(void);
void game_state_set_over(int is_over);
void game_state_check_game_over(void);

#endif // GAME_STATE_H
