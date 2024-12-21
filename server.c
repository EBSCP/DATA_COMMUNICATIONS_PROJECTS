#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h> //timeout için 
#include <ws2tcpip.h>
#include "game.h"


#pragma comment(lib, "ws2_32.lib")  // Winsock kütüphanesi

#define PORT 8080
#define BUFFER_SIZE 1024


int main() {
    WSADATA wsaData;
    SOCKET server_fd, client_sockets[MAX_PLAYERS];
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *colors[MAX_PLAYERS] = {"Red", "Blue", "Green", "Yellow"};
    int player_count = 0;
    int current_turn = 0;
    Player players[MAX_PLAYERS]; // Oyuncu bilgilerini tutan dizi
    int game_over;

    // Oyuncuları başlat
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i].id = i + 1;
        players[i].score = 0;
        players[i].roll = 0;
    }


    // 1. Winsock başlat
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // 2. Socket oluştur
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // 3. Adres ve port bilgilerini ayarla
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 4. Bağla (Bind)
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // 5. Dinlemeye başla
    if (listen(server_fd, MAX_PLAYERS) == SOCKET_ERROR) {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Server is waiting for %d players...\n", MAX_PLAYERS);

    // Renklerin durumuna göre oyuncuları kabul ediyoruz
    int color_taken[MAX_PLAYERS] = {0};  // 0 means color is available
    for (int i = 0; i < MAX_PLAYERS; i++) {
        client_sockets[i] = INVALID_SOCKET;
    }
    
    // 6. Oyuncuları kabul et
    while (player_count < MAX_PLAYERS) {
        SOCKET new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket == INVALID_SOCKET) {
            printf("Accept failed. Error Code: %d\n", WSAGetLastError());
            continue;
        }

        // Oyuncunun ayrılma durumuna göre boşluğu kontrol ediyoruz
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (color_taken[i] && client_sockets[i] != INVALID_SOCKET) {
                // İstemcinin bağlı olup olmadığını kontrol et
                char test = 0;
                int result = send(client_sockets[i], &test, 1, 0);
                if (result == SOCKET_ERROR) {
                    printf("Player with color %s disconnected\n", colors[i]);
                    closesocket(client_sockets[i]);
                    client_sockets[i] = INVALID_SOCKET;
                    color_taken[i] = 0;
                    player_count--;
                }
            }
        }

        // Dizideki ilk boşluğa oyuncuyu yerleştir
        int color_index = 0;
        for (; color_index < MAX_PLAYERS; color_index++) {
            if (!color_taken[color_index]) break;
        }

        client_sockets[color_index] = new_socket;
        color_taken[color_index] = 1;
        
        printf("Player connected. Assigned color: %s\n", colors[color_index]);

        // Renk bilgisini gönder
        send(client_sockets[color_index], colors[color_index], strlen(colors[color_index]), 0);
        player_count++;
    }

    // Oyunu başlat
    start_game(players, &game_over);

    int round_number = 0; // Tur sayısını takip etmek için

    // 7. Oyun döngüsü
    while (1) {

        while (game_over != 1) {
            round_number++;
            for (int i = 0; i < MAX_PLAYERS; i++) {
            char buffer[BUFFER_SIZE] = {0};
            time_t start_time = time(NULL); // Başlangıç zamanını al

            // Sıradaki oyuncuya tur mesajı gönder
            snprintf(buffer, BUFFER_SIZE, "Your turn, Player %d! Press SPACE to roll the dice...\n", players[i].id);
            send(client_sockets[i], buffer, strlen(buffer), 0);

            // İstemciden ROLL mesajını bekle
            int valread = recv(client_sockets[i], buffer, BUFFER_SIZE, 0);
                if (valread <= 0) {
                    printf("Player %d disconnected. Closing connection.\n", players[i].id);
                    closesocket(client_sockets[i]);
                    client_sockets[i] = INVALID_SOCKET;
                    continue;
                }
            buffer[valread] = '\0';

            // Zaman aşımını kontrol et
            if (difftime(time(NULL), start_time) >= 10) { // 10 saniye geçtiyse
                players[i].roll = 1; // Zar olarak 1 at
                printf("Player %d did not roll in time. Automatically rolled: %d\n", players[i].id, players[i].roll);
            } 
            else {
                if (strcmp(buffer, "ROLL") == 0) {
                    roll_dice(&players[i]);
                    printf("Player %d rolled: %d\n", players[i].id, players[i].roll);
                }
            }
                // Zar sonucunu istemcilere gönder
                for (int j = 0; j < MAX_PLAYERS; j++) {
                    char roll_message[BUFFER_SIZE];
                    snprintf(roll_message, BUFFER_SIZE, "Player %d rolled: %d\n", players[i].id, players[i].roll);
                    send(client_sockets[j], roll_message, strlen(roll_message), 0);
                }
            }   

        // Tur sonuçlarını işle
        determine_winner(players, MAX_PLAYERS, round_number);
        // Oyun bitiş kontrolü
        process_round(players, &game_over);
        display_scores(players, MAX_PLAYERS);
        }
    
    
    // Oyun bittiğinde skorları yazdır
    print_final_scores(players);

    // Oyun bittiğinde kazananları ve sıralamaları istemcilere gönder
    int rankings[MAX_PLAYERS];
    calculate_winner_ranking(players, MAX_PLAYERS, rankings);

    // Server konsolunda final sıralamayı göster
    printf("\n--- Final Rankings ---\n");
    for (int i = 0; i < MAX_PLAYERS; i++) {
        printf("Player %d: Rank %d, Score: %d\n", players[i].id, rankings[i], players[i].score);
    }
    printf("----------------------\n");

    // Kazananları ve sıralamaları istemcilere gönder
    for (int i = 0; i < MAX_PLAYERS; i++) {
        char end_message[BUFFER_SIZE];

        // Eğer oyuncu kazandıysa özel mesaj gönder
        if (players[i].score >= MAX_SCORE) {
            snprintf(end_message, BUFFER_SIZE, "You are the 1st Winner! Congratulations, Player %d!\n", players[i].id);
        } else {
            // Diğer oyunculara genel sıralama bilgisini gönder
            snprintf(end_message, BUFFER_SIZE, "Game Over! Player %d: Rank %d, Score: %d. Try again!\n",
                    players[i].id, rankings[i], players[i].score);
        }

        // Mesajı gönder ve bağlantı durumunu kontrol et
        if (send(client_sockets[i], end_message, strlen(end_message), 0) == SOCKET_ERROR) {
            printf("Failed to send final message to Player %d. Connection lost.\n", players[i].id);
        }
    }
    //Bu kodu kullanmazsak server.exe sürekli döngüde kalıyor
    if (game_over) {
            break; // Exit the loop if the game is over
        }
}

    // Oyun bitişinde server.exe nin kapanmaması için 
    printf("Server is still running. Press Ctrl+C to shut down.\n");
    while (1) {
        Sleep(1000);
    }

    // Bağlantıları temizle
    for (int i = 0; i < MAX_PLAYERS; i++) {
        closesocket(client_sockets[i]);
    }
    closesocket(server_fd);
    WSACleanup();

    printf("Server shut down successfully.\n");
    return 0;
}
