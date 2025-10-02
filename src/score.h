#ifndef SCORE_H
#define SCORE_H

// Scoring system
#define SCORE_PIECE_PLACEMENT 5
#define SCORE_LINE_CLEAR_1 20
#define SCORE_LINE_CLEAR_2 40
#define SCORE_LINE_CLEAR_3 80
#define SCORE_LINE_CLEAR_4_PLUS 140

// Score management
void score_init(void);
int score_get_current(void);
void score_add_placement(void);
void score_add_points(int points);
void score_clear_lines(void);
void score_draw(void);

#endif // SCORE_H
