#pragma once

#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>

typedef struct sockaddr* clientAddress;

typedef struct Server {
    char* port;
    int listenSocket;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    int windowSize;
    clientAddress clientAddress;
} Server;

typedef Server* serverPtr;

void serverConstructor(serverPtr serverPtr);
void serverDestructor(serverPtr serverPtr);
void setWindowSize(serverPtr serverPtr, int windowSize);
int getWindowSize(serverPtr serverPtr);
int getListenSocket(serverPtr serverPtr);
void recieve(serverPtr serverPtr);