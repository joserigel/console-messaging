#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <conio.h>


#define DEFAULT_BUFLEN 512
/*
    Required libraries {g++}: 
    -lwsock32
    -lWs2_32
*/

HANDLE hConsole;
void gotoxy(byte x, byte y) {
    SetConsoleCursorPosition(hConsole, {x, y});
}

bool retry() {
    printf("Try again?[Y/N]");
    char input = getch();
    printf("\n");
    if (input == 'N' || input == 'n') return false;
    else return true;
}

int beServer() {
    //initializing of hints and results, but mostly hints
    struct addrinfo *result = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    //Get address info based on hints
    int iResult;
    while (true) {
        iResult = getaddrinfo(NULL, "8080", &hints, &result);
        if (iResult != 0) {
            printf("Getting address info failed at port 8080");
            if (!retry()) return 1;
            else continue;
        }
        break;
    }
    system("CLS");

    //Create ListenSocket
    SOCKET ListenSocket = INVALID_SOCKET;
    while (true) {
        ListenSocket = socket(
            result->ai_family,
            result->ai_socktype,
            result->ai_protocol
        );

        if (ListenSocket == INVALID_SOCKET) {
            printf("Error at socket(0: %ld\n", WSAGetLastError());
            if (!retry()) return 1;
            else continue;
        }
        break;
    }

    //Bind Socket
    while (true) {
        iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

        if (iResult == SOCKET_ERROR) {
            printf("Socket bind failed: %d\n", WSAGetLastError());
            if (!retry()) {
                freeaddrinfo(result);
                closesocket(ListenSocket);
                WSACleanup();
                return 1;
            } else continue;
        }
        break;
    }
    system("CLS");
    freeaddrinfo(result);
    
    //Listening at Socket
    while (true) {
        char ipv4[INET_ADDRSTRLEN];    
        inet_ntop(AF_INET, &result->ai_addr, ipv4, INET_ADDRSTRLEN);
        printf("Currently listening at: %s\n", ipv4);

        if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
            printf("Listening failed with error: %ld\n", WSAGetLastError());
            if (!retry()) {
                WSACleanup();
                return 1;
            } else continue;
        }
        break;
    }
    
    //Accept Client 
    char client_ip[INET_ADDRSTRLEN];
    SOCKET ClientSocket = INVALID_SOCKET;
    while (true) {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        getpeername(ClientSocket, result->ai_addr, (int *)&result->ai_addrlen);
        inet_ntop(AF_INET, &result->ai_addr, client_ip, INET_ADDRSTRLEN);
        printf("Accepted Client from: %s", client_ip);
        if (ClientSocket == INVALID_SOCKET) {
            printf("Accept failed: %d\n", WSAGetLastError());
            if (!retry()) {
                closesocket(ListenSocket);
                WSACleanup();
                return 1;
            } else continue;
        }
        break;
    }
    closesocket(ListenSocket);
    system("CLS");


    //Receive and Send Datas
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iSendResult;

    do {
        if (kbhit()) {
            char inp = getch();
            if (inp == 'q' || inp == 'Q') break;
        }

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("[%s]: %s", client_ip, recvbuf);

            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf(";echo failed\n");
            } else {
                printf("\n");
            }
        } else if (iResult < 0) {
            printf("An error occured: %ld\n", WSAGetLastError());
            break;
        }
    } while(true);
    
    //Shutdown and Cleanup
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) printf("Shutdown failed: %d\n", WSAGetLastError());
    
    closesocket(ClientSocket);
    return 0;
}

int beClient() {
    
}

int main() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    //Start of WSA
    WSAData wsa;
    int iResult;
    while(true) {
        iResult = WSAStartup(MAKEWORD(2,2), &wsa);
        if (iResult != 0) {
            printf("WSA Startup Failed: %d\n", iResult);
            if (!retry()) return 1;
            else continue;
        }
        break;
    }
    system("CLS");

    int exitCode = beServer();
    WSACleanup();
    system("PAUSE");
    return exitCode;
}