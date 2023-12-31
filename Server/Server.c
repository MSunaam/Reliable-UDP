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
void serverConstructor(serverPtr serverPtr);
void serverDestructor(serverPtr serverPtr);
void setWindowSize(serverPtr serverPtr, int windowSize);
int getWindowSize(serverPtr serverPtr);
int getListenSocket(serverPtr serverPtr);
void recieve(serverPtr serverPtr);

void serverConstructor(serverPtr serverPtr){

    // Window Size
    setWindowSize(serverPtr, 5);

    // Port
    serverPtr->port = "8000";

    // Reset hints struct
    memset(&(serverPtr->hints), sizeof serverPtr->hints, 0);
    // Filling in the hints addrinfo
    serverPtr->hints.ai_family = AF_INET; // IPv4
    serverPtr->hints.ai_socktype = SOCK_DGRAM; //UDP Socket
    serverPtr->hints.ai_flags =  AI_PASSIVE; // Fill in my IP for me

    int status;

    if((status = getaddrinfo(NULL, (char*)serverPtr->port, &(serverPtr->hints), &(serverPtr->servinfo))) < 0){
        perror("\033[91mServer GetAddrInfo():\033[0m");
        exit(EXIT_FAILURE);
    }

    // Find correct node in servinfo linked list
    for(struct addrinfo *ptr = serverPtr->servinfo; ptr != NULL; ptr = ptr->ai_next){
        // Open a socket
        if((serverPtr->listenSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1){
            perror("\033[91mServer UDP socket():\033[0m");
            // Look for next valid node
            continue;
        }
        // Bind to the socket
        if(bind(serverPtr->listenSocket, ptr->ai_addr, ptr->ai_addrlen) < 0){
            // Close socket
            close(serverPtr->listenSocket);
            perror("\033[91mServer UDP bind():\033[0m");
            // Look for next valid node
            continue;
        }
        // If valid node found and socket() and bind() calls are successful
        break;
    }
    // Server ready to listen
    printf("\033[92mServer listening at port %s...\n\033[0m", serverPtr->port);
}
void serverDestructor(serverPtr serverPtr){
    // Free servinfo
    freeaddrinfo(serverPtr->servinfo);
}
void setWindowSize(serverPtr serverPtr, int windowSize){
    serverPtr->windowSize = windowSize;
    return;
}
int getWindowSize(serverPtr serverPtr){
    return serverPtr->windowSize;
}
int getListenSocket(serverPtr serverPtr){
    return serverPtr->listenSocket;
}
void recieve(serverPtr serverPtr){
    // Video File Parameters
    size_t bytesRecieved = 0;
    size_t fileSize = 0;
    size_t remainingBytes = 0;

    // Packets array
    Packet packetArray[getWindowSize(serverPtr)];

    // Recieve Request From Client
    socklen_t addrLen = sizeof (struct sockaddr);
    char fileName[MAX_BUFFER_SIZE];
    if((bytesRecieved = recvfrom(
        getListenSocket(serverPtr),
        &fileName,
        sizeof fileName,
        0,
        (struct sockaddr *)serverPtr->clientAddress,
        &addrLen
        )) < 0){
        perror("\033[91mServer recvFrom():\033[0m");
        exit(EXIT_FAILURE);
    }
}