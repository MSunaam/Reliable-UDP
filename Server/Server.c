#include "Server.h"
#include "../Packet/packet.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

#define MAX_BUFFER_SIZE 500

// Reset Code =>  \033[0m
// Red Error String => \033[91m
// Green success code => \033[92m
// Blue Info code => \033[94m
// Bold Code => \033[1m

#define WINDOW_SIZE 5

// Function Prototypes
void serverConstructor(serverPtr server);
void serverDestructor(serverPtr server);
void setWindowSize(serverPtr server, int windowSize);
int getWindowSize(serverPtr server);
int getListenSocket(serverPtr server);
void receiveFile(serverPtr server);
void* receivePackets(void* params);

// Packets array
Packet packetArray[WINDOW_SIZE];
// Acknowledgements
int ackArray[WINDOW_SIZE];
int numberAcks;
int currentSequenceNumber;
int no_of_packets = 5;

socklen_t addrLen = sizeof (struct sockaddr);

void serverConstructor(serverPtr server){

    // Window Size
    setWindowSize(server, WINDOW_SIZE);

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

    // Receive Datagrams
    receiveFile(server);
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
void receiveFile(serverPtr server){
    // Video File Parameters
    size_t bytesReceived = 0;
    size_t fileSize = 0;
    size_t remainingBytes = 0;
    char fileName[MAX_BUFFER_SIZE];

    // Multithreading Parameters
    pthread_t threadId;
    ThreadParams params;

    // Time delay variables
	struct timespec reqTime,remTime;
	reqTime.tv_sec = 0;
	reqTime.tv_nsec = 30000000L;  // 0.03 seconds

    // Receive File Name From Client
    memset(&(server->clientAddress), 0, addrLen);
    memset(&fileName,0,sizeof fileName);

    if((bytesReceived = recvfrom(
        getListenSocket(server),
        &fileName,
        sizeof fileName,
        0,
        (struct sockaddr*)&server->clientAddress,
        &addrLen
        )) < 0){
        perror("\033[91mServer recvfrom():");
        printf("\033[0m");
        exit(EXIT_FAILURE);
    }

    // Print Client's address
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(
        AF_INET,
        ((struct sockaddr_in*)&(server->clientAddress)),
        clientIP,
        INET_ADDRSTRLEN);
    int port =  ntohs(((struct sockaddr_in *)&(server->clientAddress))->sin_port);
    printf("\033[94mConnected to Client %s:%d\n\033[0m",clientIP, port);

    // Print Filename
    printf("\033[1mFilename: %s\n\033[0m", fileName);

    // Get Filesize
    if((bytesReceived = recvfrom(
        getListenSocket(server),
        &fileSize,
        sizeof (fileSize),
        0,
        (struct sockaddr*)&server->clientAddress,
        &addrLen
        )) < 0){
        perror("\033[91mServer recvfrom():");
        printf("\033[0m");
        exit(EXIT_FAILURE);
    }

    // Print FileSize
    printf("\033[1mFile Size: %ld Bytes\n\033[0m",fileSize);

    // Open File
    FILE* file = fopen(fileName, "wb");

    // Receive File
    remainingBytes = fileSize;
    bytesReceived = 0;

    while (remainingBytes > 0 && no_of_packets == 5){
        // Reset the arrays
        memset(packetArray, 0, sizeof packetArray);
        for (int i = 0; i < no_of_packets; i++){
            packetArray[i].packetSize = 0;
            ackArray[i] = 0;
        }

        // Receive packets in another thread
        memset(&params,0,sizeof params);
        params.server = server;
        pthread_create(&threadId, NULL, receivePackets, (void*)&(params));

        // Wait for packets to start arriving
        nanosleep(&reqTime, &remTime);
        numberAcks = 0;

        // Keep running while all packets in one window are not received
        RESEND_ACK:
        for(int i =0; i < no_of_packets; i++){
            int currentAck = packetArray[i].packetSequenceNumber;
            // If ack not already sent
            if(ackArray[currentAck] != 1){
                // Send acks of received packets
                if(packetArray[i].packetSize != 0){
                    ackArray[currentAck] = 1;
                    // Send Acks
                    if(sendto(getListenSocket(server), &currentAck, sizeof(currentAck), 0, (struct sockaddr *)&server->clientAddress, addrLen) > 0) {
						numberAcks++;
						printf("Ack sent: %d\n", currentAck);
					}
                }
            }
        }

        // Stop and Wait for acks to be received
        nanosleep(&reqTime, &remTime);
        nanosleep(&reqTime, &remTime);

        // If all packets not received
        if(numberAcks < no_of_packets){
            goto RESEND_ACK;
        }

        // Window Size number of packets have been received
        pthread_join(threadId, NULL);

        // Write packets to the output file
        for(int i = 0; i < no_of_packets; i++){
            // Check for last packet which will have size -1
            if(packetArray[i].packetSize > 0){
                printf("Writing packet: %d\n", packetArray[i].packetSequenceNumber);
                // Write to file
                fwrite(packetArray[i].packetData, 1, packetArray[i].packetSize, file);
                // Updating stats
                remainingBytes -= packetArray[i].packetSize;
                bytesReceived += packetArray[i].packetSize;
            }
        }

        printf("\033[1mSize Received: %ld bytes\t Size Remaining: %ld bytes\n\033[0m", bytesReceived, remainingBytes);
        // Loop for next 5 packets
        
    }
    // Received All Packets
    printf("\033[1m\033[92mFile Received and Saved\033[0m\n");
    printf("Closing Socket\n");
    close(getListenSocket(server));
    fclose(file);
}
void* receivePackets(void* params){
    ThreadParams *param = (ThreadParams*)params;
    serverPtr server = param->server;

    // Stats
    int receivedBytes = 0;
    Packet currentPacket;
    memset(&currentPacket, 0, sizeof currentPacket);

    // Receive Window Size number of packets
    for(int i = 0; i < no_of_packets; i++)
    {
        RECEIVE:
        if((receivedBytes = recvfrom(getListenSocket(server), &currentPacket, sizeof currentPacket, 0, (struct sockaddr*)&server->clientAddress, &addrLen))<0){
            perror("\033[91mServer recvfrom:\033[0m");
            exit(EXIT_FAILURE);
        }
        // For duplicate packet
        if(packetArray[currentPacket.packetSequenceNumber].packetSize != 0){
            // Reset Array
            packetArray[currentPacket.packetSequenceNumber] = currentPacket;

            int currentAck = currentPacket.packetSequenceNumber;
            ackArray[currentAck] = 1;
            // Send Diplicate ACK
            if(sendto(getListenSocket(server), &currentAck, sizeof currentAck, 0, (struct sockaddr*)&server->clientAddress, addrLen)<0){
                perror("\033[91mServer sendto:\033[0m");
                exit(EXIT_FAILURE);
            }
            printf("\033[94mDuplicate ACK Sent:%d\n", currentAck);
            goto RECEIVE;
        }

        //For Last Packet
        if(currentPacket.packetSize == -1){
            printf("\033[92mLast Packet Found\n\033[0m");
            no_of_packets = currentPacket.packetSequenceNumber + 1;
        } 
        //For Unique Packet
        if (receivedBytes > 0) {
			printf("Packet Received:%d\n", currentPacket.packetSequenceNumber);
			// Keep the correct order of packets by index of the array
			packetArray[currentPacket.packetSequenceNumber] = currentPacket;
		}
    }
    return NULL;
}
