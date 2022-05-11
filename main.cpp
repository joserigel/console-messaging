#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <conio.h>

/*
    Required libraries {g++}: 
    -lwsock32
    -lWs2_32
*/

HANDLE hConsole;
void gotoxy(byte x, byte y) {
    SetConsoleCursorPosition(hConsole, {x, y});
}

int main() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WSAData wsa;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsa);
    while (iResult != 0) {
        printf("WSA Startup Failed: %d\n", iResult);
        printf("Try again?[Y/N]");
        char input = getch();
        if (input == 'N' || input == 'n') return 1;
    }

    //initializing of hints and result
    struct addrinfo 
        *result = NULL, 
        hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, "8000", &hints, &result);



    ZeroMemory(&hConsole, sizeof(hConsole));
    WSACleanup();
    
    return 0;
}