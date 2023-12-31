#include "Server.h"
#include "../Packet/packet.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_BUFFER_SIZE 500

// Reset Code =>  \033[0m
// Red Error String => \033[91m
// Green success code => \033[92m

// Function Prototypes
void serverConstructor(serverPtr server);
void serverDestructor(serverPtr server);
void setWindowSize(serverPtr server, int windowSize);
int getWindowSize(serverPtr server);
int getListenSocket(serverPtr server);
void recieve(serverPtr server);

void serverConstructor(serverPtr server){

    // Window Size
    setWindowSize(server, 5);

    // Port
    server->port = "8000";

    // Reset hints struct
    memset(&(server->hints), 0,sizeof server->hints);
    // Filling in the hints addrinfo
    server->hints.ai_family = AF_UNSPEC; // IPv4 or 6
    server->hints.ai_socktype = SOCK_DGRAM; //UDP Socket
    server->hints.ai_flags =  AI_PASSIVE; // Fill in my IP for me

    int status;

    if((status = getaddrinfo(NULL, (char*)server->port, &(server->hints), &(server->servinfo))) != 0){
        fprintf(stderr, "\033[91mgetaddrinfo error: %s\n\033[0m", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Find correct node in servinfo linked list
    for(struct addrinfo *ptr = server->servinfo; ptr != NULL; ptr = ptr->ai_next){
        // Open a socket
        if((server->listenSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1){
            perror("\033[91mServer UDP socket():");
            printf("\033[0m");
            // Look for next valid node
            continue;
        }
        // Bind to the socket
        if(bind(server->listenSocket, ptr->ai_addr, ptr->ai_addrlen) < 0){
            // Close socket
            close(server->listenSocket);
            perror("\033[91mServer UDP bind():");
            printf("\033[0m");
            // Look for next valid node
            continue;
        }
        // If valid node found and socket() and bind() calls are successful
        break;
    }
    // Server ready to listen
    printf("\033[92mServer listening at port %s...\n\033[0m", server->port);

    // Recieve Datagrams
    recieve(server);
}
void serverDestructor(serverPtr server){
    // Free servinfo
    freeaddrinfo(server->servinfo);
}
void setWindowSize(serverPtr server, int windowSize){
    server->windowSize = windowSize;
    return;
}
int getWindowSize(serverPtr server){
    return server->windowSize;
}
int getListenSocket(serverPtr server){
    return server->listenSocket;
}
void recieve(serverPtr server){
    // Video File Parameters
    size_t bytesRecieved = 0;
    size_t fileSize = 0;
    size_t remainingBytes = 0;

    // Packets array
    Packet packetArray[getWindowSize(server)];

    // Recieve Request From Client
    socklen_t addrLen = sizeof (struct sockaddr);
    char fileName[MAX_BUFFER_SIZE];
    if((bytesRecieved = recvfrom(
        getListenSocket(server),
        &fileName,
        sizeof fileName,
        0,
        (struct sockaddr *)server->clientAddress,
        &addrLen
        )) < 0){
        perror("\033[91mServer recvFrom():");
        printf("\033[0m");
        exit(EXIT_FAILURE);
    }

    // Print Client's address
}