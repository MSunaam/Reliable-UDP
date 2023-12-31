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

void serverConstructor(serverPtr server);
void serverDestructor(serverPtr server);
void setWindowSize(serverPtr server, int windowSize);
int getWindowSize(serverPtr server);
int getListenSocket(serverPtr server);
void recieve(serverPtr server);