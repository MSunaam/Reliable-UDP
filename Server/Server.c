#include "Server.h"
#include "../Packet/packet.h"
//necessary header files required for network communication, file operations, threading, and other functionalities.
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
// MAX_BUFFER_SIZE specifies the maximum size of the buffer for sending and receiving data. 
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
//ServerConstructor uses  the getaddrinfo function to retrieve address information for the server, based on the hints provided.
//It loops through the resulting linked list of addrinfo structures and tries to create a socket using the socket function. 
// If an error occurs, it continues to the next node in the list.
//If a socket can be created, it binds the socket to the address using the bind function. 
//After a  valid node is found and the socket is successfully created and bound, it breaks out of the loop and proceeds to 
//receive datagrams by calling the receiveFile function.
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
    // It frees the memory allocated for the servinfo structure using the freeaddrinfo function.
    freeaddrinfo(server->servinfo);
}

//The setWindowSize function sets the window size of the server.
// The getWindowSize function returns the window size of the server.
// The getListenSocket function returns the listen socket of the server.
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
    size_t bytesReceived = 0; //This variable will be used to store the number of bytes received during the file transfer.
    size_t fileSize = 0; //This variable will be used to store the total size of the file being received.
    size_t remainingBytes = 0; //This variable will be used to keep track of the number of bytes remaining to be received.
    char fileName[MAX_BUFFER_SIZE];

    // Multithreading Parameters
    pthread_t threadId;
    ThreadParams params; //ThreadParams is a custom struct that holds parameters for the thread function. It will be used to pass parameters to the thread function.

    // Time delays for nanosleep function
	struct timespec reqTime,remTime;
	reqTime.tv_sec = 0;
	reqTime.tv_nsec = 30000000L;  // 0.03 seconds

    // Receive File Name From Client:
	
    //clears the memory of the clientAddress member of the server object
    memset(&(server->clientAddress), 0, addrLen);
    //clears the memory of the fileName array, setting all elements to 0.
    memset(&fileName,0,sizeof fileName);
	
    //It receives filename from the socket associated with the server object's listening socket (getListenSocket(server))
    if((bytesReceived = recvfrom(getListenSocket(server),&fileName,sizeof fileName,0,(struct sockaddr*)&server->clientAddress,&addrLen)) < 0)
    {
        perror("\033[91mServer recvfrom():");
        printf("\033[0m");
        exit(EXIT_FAILURE);
    }

    // Print Client's address :   a string representation (clientIP and port) using the inet_ntop function.
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,((struct sockaddr_in*)&(server->clientAddress)), clientIP,INET_ADDRSTRLEN);
    int port =  ntohs(((struct sockaddr_in *)&(server->clientAddress))->sin_port);
    printf("\033[94mConnected to Client %s:%d\n\033[0m",clientIP, port);

    // Print Filename:
    printf("\033[1mFilename: %s\n\033[0m", fileName);

    // Receive File Size From Client ( in same way as File name):
    if((bytesReceived = recvfrom(getListenSocket(server),&fileSize,sizeof (fileSize),0,(struct sockaddr*)&server->clientAddress,&addrLen)) < 0)
    {
        perror("\033[91mServer recvfrom():");
        printf("\033[0m");
        exit(EXIT_FAILURE);
    }

    // Print FileSize:
    printf("\033[1mFile Size: %ld Bytes\n\033[0m",fileSize);

    // Open File
    FILE* file = fopen(fileName, "wb");

    // Receive File
    remainingBytes = fileSize; //indicating that all the bytes of the file are remaining to be received yet.
    bytesReceived = 0; // indicating that no bytes have been received yet.

//loop that will continue until all the bytes of the file have been received and no_of_packets is equal to 5.
    while (remainingBytes > 0 && no_of_packets == 5)
    {
        // clear the memory of the packetArray 
        memset(packetArray, 0, sizeof packetArray);
	    
	// clear the memory of the ackArray array
        for (int i = 0; i < no_of_packets; i++){
            packetArray[i].packetSize = 0;
            ackArray[i] = 0;
        }

        memset(&params,0,sizeof params);
        params.server = server;
	//new thread (RECEIVE PACKETS THREAD) -> to receive packets 
        pthread_create(&threadId, NULL, receivePackets, (void*)&(params));

        // Wait for packets to start arriving
        nanosleep(&reqTime, &remTime);
	//numberAcks is a variable that keeps track of the number of acknowledgments received.
        numberAcks = 0;

        // Keep running while all packets in one window are not received
        RESEND_ACK:
	
	//for loop that iterates over each packet in the packetArray array
        for(int i =0; i < no_of_packets; i++){
            int currentAck = packetArray[i].packetSequenceNumber;
	    //If the packet has been received (packetArray[i].packetSize != 0), it marks the packet as acknowledged (ackArray[currentAck] = 1)::
		
            //1. If ack not already sent
            if(ackArray[currentAck] != 1){
                // 2.Send acks of received packets
                if(packetArray[i].packetSize != 0){
                    ackArray[currentAck] = 1;
                    // Send Acks usinf sendto function
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
	//the thread is joined using pthread_join to synchronize the main thread with the RECEIVE PACKETS thread.
        pthread_join(threadId, NULL);

        // Write packets to the output file:
	
        for(int i = 0; i < no_of_packets; i++){
            // checks if the packet is not the last packet (packetArray[i].packetSize > 0).
            if(packetArray[i].packetSize > 0){
		// print message
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

//THREAD FUNCTION ->  to RECIEVE PACKETS.
void* receivePackets(void* params){
    ThreadParams *param = (ThreadParams*)params;
    serverPtr server = param->server; //The param pointer is assumed to be a pointer to a ThreadParams struct, and server is a member of that struct

    // Stats
    int receivedBytes = 0;
    Packet currentPacket;
    memset(&currentPacket, 0, sizeof currentPacket);

    // Receive Window Size number of packets
    for(int i = 0; i < no_of_packets; i++)
    {
        RECEIVE:
	//receive a packet from the client using the recvfrom function. The received packet is stored in the currentPacket variable
        if((receivedBytes = recvfrom(getListenSocket(server), &currentPacket, sizeof currentPacket, 0, (struct sockaddr*)&server->clientAddress, &addrLen))<0){
            perror("\033[91mServer recvfrom:\033[0m");
            exit(EXIT_FAILURE);
        }
        // DUPLICATE PACKET : checks if the packet with the same sequence number (currentPacket.packetSequenceNumber) has already been received
        if(packetArray[currentPacket.packetSequenceNumber].packetSize != 0)
	// If it is a duplicate packet, update the packetArray with the new packet, 
	//mark the packet as acknowledged (ackArray[currentAck] = 1), 
	//and send a duplicate acknowledgment back to the client using the sendto function.
	{
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

        //LAST PACKET : checks if the packetSize of the currentPacket is -1, indicating that it is the last packet.
        if(currentPacket.packetSize == -1)
	//f it is the last packet, it updates the no_of_packets variable to the packet's sequence number plus 1, 
	//indicating the total number of packets received.
	{
            printf("\033[92mLast Packet Found\n\033[0m");
            no_of_packets = currentPacket.packetSequenceNumber + 1;
        } 
        //UNIQUE PACKET:If the receivedBytes (number of bytes received) is greater than 0, 
	// it prints a message indicating the packet number received and updates the packetArray with the received packet.
        if (receivedBytes > 0) {
			printf("Packet Received:%d\n", currentPacket.packetSequenceNumber);
			// Keep the correct order of packets by index of the array
			packetArray[currentPacket.packetSequenceNumber] = currentPacket;
		}
    }
//thread function returns NULL to indicate the completion of its execution.

    return NULL;
}
