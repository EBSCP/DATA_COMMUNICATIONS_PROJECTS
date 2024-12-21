#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define MAX_PLAYERS 4
#define MAX_SCORE 4

typedef struct {
    int id;
    int score;
    int roll;
} Player;

//Her tur sonuncundaki skor tablosunu gösteriyor
void display_scores(Player players[], int num_players) {
    printf("\n--- Score Table ---\n");
    for (int i = 0; i < num_players; i++) {
        printf("Player %d: %d points\n", players[i].id, players[i].score);
    }
    printf("-------------------\n\n");
}

int determine_winner(Player players[], int num_players, int round_number) {
    int max_roll = 0;  // En yüksek zar değeri başlangıç değerini atıyorum
    int winners[MAX_PLAYERS];  // Kazananların indekslerini tutacak
    int winner_count = 0;

    // 1. Oyuncuların zarlarını karşılaştırarak en yüksek değeri bul
    for (int i = 0; i < num_players; i++) {
        if (players[i].roll > max_roll) {
            max_roll = players[i].roll;  // En yüksek zar değerini güncelle
        }
    }

    // 2. En yüksek zar atan oyuncuları belirle
    for (int i = 0; i < num_players; i++) {
        if (players[i].roll == max_roll) {
            winners[winner_count++] = i;  // Kazanan oyuncunun indeksini kaydet
        }
    }

    // 3. Kazananlara puan ekle
    for (int i = 0; i < winner_count; i++) {
        players[winners[i]].score++;  // Skoru artır
    }

    // 4. Round sonuçlarını server konsoluna yazdır
    printf("\n--- %d. Round Summary ---\n", round_number);
    printf("Highest Roll: %d\n", max_roll);
    printf("Round Winners: ");
    for (int i = 0; i < winner_count; i++) {
        printf("Player %d ", players[winners[i]].id);
    }
    printf("\n---------------------\n");

    return winner_count;
}


//10 skoruna ulaşılıp ulaşılmadığını kontrol et
int check_game_over(Player players[], int num_players) {
    for (int i = 0; i < num_players; i++) {
        if (players[i].score >= MAX_SCORE) {
            return 1;
        }
    }
    return 0;
}

//Oyuncuların skorlarını ve zar değerleri için
void initialize_players(Player players[], int num_players) {
    for (int i = 0; i < num_players; i++) {
        players[i].id = i + 1;
        players[i].score = 0;
        players[i].roll = 0;
    }
}

void roll_dice(Player *player) {
    player->roll = (rand() % 6) + 1;
}

void calculate_winner_ranking(Player players[], int num_players, int rankings[]) {
   for (int i = 0; i < num_players; i++) {
        rankings[i] = 1;
        for (int j = 0; j < num_players; j++) {
            if (players[j].score > players[i].score) {
                rankings[i]++;
            }
        }
    }
}

void start_game(Player players[], int *game_over) {
    srand(time(NULL));
    *game_over = 0;
    initialize_players(players, MAX_PLAYERS);
}

void process_round(Player players[], int *game_over) {
    // Her turun başında oyuncuların zar değerlerini sıfırla
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i].roll = 0;
    }

    // Oyuncular zar atsın
    for (int i = 0; i < MAX_PLAYERS; i++) {
        roll_dice(&players[i]);
    }

    // Oyunun bitip bitmediğini kontrol et
    *game_over = check_game_over(players, MAX_PLAYERS);
}


void print_final_scores(Player players[]) {
    printf("\nGame Over! Final Scores:\n");
    display_scores(players, MAX_PLAYERS);

    int rankings[MAX_PLAYERS];
    calculate_winner_ranking(players, MAX_PLAYERS, rankings);

    printf("\nWinner(s):\n");
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].score >= MAX_SCORE) {
            printf("Player %d (Rank: %d)\n", players[i].id, rankings[i]);
        }
    }

    printf("\nOverall Rankings:\n");
    for (int i = 0; i < MAX_PLAYERS; i++) {
        printf("Player %d: Rank %d, Score: %d\n", players[i].id, rankings[i], players[i].score);
    }
    printf("\n");
}



