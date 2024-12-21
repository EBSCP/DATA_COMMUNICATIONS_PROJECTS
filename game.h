#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>

#define MAX_PLAYERS 4
#define MAX_SCORE 4

typedef struct {
    int id;
    int score;
    int roll;
} Player;

void display_scores(Player players[], int num_players);
int determine_winner(Player players[], int num_players, int round_number);
int check_game_over(Player players[], int num_players);
void initialize_players(Player players[], int num_players);
void roll_dice(Player *player);
void calculate_winner_ranking(Player players[], int num_players, int rankings[]);
void start_game(Player players[], int *game_over);
void process_round(Player players[], int *game_over);
void print_final_scores(Player players[]);

#endif // GAME_H

