#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>  // _kbhit ve getch için


#pragma comment(lib, "ws2_32.lib")  // Winsock kütüphanesi

#define PORT 8080
#define SERVER_IP "10.57.37.203"  // Yerel IP adresiniz
#define BUFFER_SIZE 1024


int main() {
    WSADATA wsaData;
    SOCKET client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE] = {0};

    // 1. Winsock başlat
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // 2. Socket oluştur
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // 3. Sunucu adresini ayarla
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr);

    // 4. Sunucuya bağlan
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("Connection to the server failed. Error Code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    printf("Connected to the server. Waiting for color assignment...\n");

    // 5. Renk bilgisini al
    int valread = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (valread > 0) {
        buffer[valread] = '\0';
        printf("Your assigned color: %s\n", buffer);
    }

    // 6. Sunucudan mesajları dinle
    while ((valread = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[valread] = '\0';  // Gelen mesajı sonlandır
        printf("%s", buffer);    // Mesajı ekrana yazdır

        // Mesaj "Your turn" ise zar atmak için bekle
        if (strstr(buffer, "Your turn") != NULL) {
            printf("Press SPACE to roll the dice...\n");

            // Zar atmak için SPACE tuşuna basılmasını bekle
            while (1) {
                if (_kbhit() && _getch() == ' ') {
                    char roll_message[] = "ROLL";
                    send(client_socket, roll_message, strlen(roll_message), 0);
                    printf("You rolled the dice!\n");
                    break;
                }
            }
        }

        // Oyun bitiş mesajlarını algıla
        if (strstr(buffer, "Winner") != NULL || strstr(buffer, "Game Over") != NULL) {
            printf("\n%s\n", buffer);  // Final mesajını ekrana yazdır
            break;  // Oyun bitiş mesajı geldiyse döngüyü sonlandır
        }
    }


    // Bağlantı kesildiğinde veya oyun bittiğinde
    if (valread <= 0) {
        printf("Disconnected from server. Exiting...\n");
    }
    else {
        // Server'dan gelen mesaj "Game Over" ise client'ı kapatmadan beklet
        printf("Press any key to exit...\n");
        _getch(); // Kullanıcıdan bir tuşa basmasını bekle
    }

    closesocket(client_socket);
    WSACleanup();

    return 0;
}
