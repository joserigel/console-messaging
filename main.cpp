#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <conio.h>


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8080"
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
        iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
            printf("Getting address info failed at port %s", DEFAULT_PORT);
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
    system("CLS");

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
        
        struct sockaddr_in s_in;
        socklen_t socklen = sizeof(s_in);
        char ipv4[INET_ADDRSTRLEN];
        if (getsockname(ListenSocket, (struct sockaddr *) &s_in, &socklen) != -1) {
            inet_ntop(AF_INET, &s_in.sin_addr, ipv4, INET_ADDRSTRLEN);
            printf("Listening at: %s", ipv4);
        }


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
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        ClientSocket = accept(ListenSocket, (sockaddr*) &addr, &addrlen);
        
        inet_ntop(AF_INET, &addr.sin_addr, client_ip, INET_ADDRSTRLEN);

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
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("[%s]: %s ", client_ip, recvbuf);

            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("; Echo failed\n");
            } else {
                printf("\n");
            }
            if (strcmp(recvbuf, "!quit") == 0) break;
        } else if (iResult < 0) {
            printf("An error occured: %ld\n", WSAGetLastError());
            break; 
        }
        
    } while(true);
    
    //Shutdown and Cleanup
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) 
        printf("Shutdown failed: %d\n", WSAGetLastError());
    
    closesocket(ClientSocket);
    return 0;
}

int beClient() {
    //Initializing of hints (+result pointer)
    struct addrinfo *result = NULL, hints; 
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    //Get address info
    int iResult;
    while (true) {
        char ipv4[INET_ADDRSTRLEN];
        printf("IP: ");
        std::cin.getline(ipv4, sizeof(ipv4));
        iResult = getaddrinfo(ipv4, DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
            printf("getaddrinfo failed: %d\n", iResult);
            if (!retry()) {
                WSACleanup();
                return 1;
            } else continue;
        }
        break;
    }
    system("CLS");

    //Establish Connection with a server
    SOCKET ConnectSocket = INVALID_SOCKET;
    for (struct addrinfo *ptr=result; true; ptr=ptr->ai_next) {
        if (ptr == NULL) {
            printf("No valid socket can be found\n");
            return 1;
        }
        ConnectSocket = socket(
            ptr->ai_family,
            ptr->ai_socktype,
            ptr->ai_protocol
        );

        if (ConnectSocket == INVALID_SOCKET) {
            printf("\nSocket failed: %ld\n", WSAGetLastError());
            system("PAUSE");
            continue;
        }
        
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to Connect to server\n");
        WSACleanup();
        system("PAUSE");
        return 1;
    }

    //Printing of Server/Client IP
    struct sockaddr_in sin;
    socklen_t sin_len = sizeof(sin);
    if (getsockname(ConnectSocket, (struct sockaddr*)&sin, &sin_len) != -1) {
        char ipv4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sin.sin_addr, ipv4, INET_ADDRSTRLEN);
        printf("Connected from: %s\n", ipv4);
    }

    if (getpeername(ConnectSocket, (struct sockaddr*)&sin, &sin_len) != -1) {
        char ipv4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sin.sin_addr, ipv4, INET_ADDRSTRLEN);
        printf("Connected To: %s\n", ipv4);
    }


    //Send and receive data
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    int bufsize = DEFAULT_BUFLEN;
    while (true) {
        printf("Msg: ");
        std::cin.getline(sendbuf, sizeof(sendbuf));
        iResult = send(ConnectSocket, sendbuf, bufsize, 0);
        if (iResult == SOCKET_ERROR) {
            printf("SEND FAILED!!! %d\n", WSAGetLastError());
            break;
        }
        
        iResult = recv(ConnectSocket, recvbuf, bufsize, 0);
        if (iResult > 0) {
            printf("Message Received! ");
            printf((strcmp(sendbuf, recvbuf)==0)? "\n" : "(with error)\n");
        } else if (iResult == 0) {
            printf("Message not received\n");
            break;
        } else {
            printf("Recv failed: %d\n", WSAGetLastError());
            break;
        }

        if (strcmp(sendbuf, "!quit") == 0) break;
    }

    //Shutdown and Cleanup
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) 
        printf("shutdown failed: %d\n", WSAGetLastError());

    closesocket(ConnectSocket);
    return 0;
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

    //User option prompt
    char input;
    do {
        system("CLS");
        printf("[C]lient or [S]erver?");
        input = getch();
        printf("\n");
    } while (input != 'C' && input != 'c' && 
        input != 'S' && input != 's');
    system("CLS");

    int exitCode = (input == 'C' || input == 'c')? beClient() : beServer();
   
    WSACleanup();
    return exitCode;
}