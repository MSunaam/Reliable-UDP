#include "Client.h"
#include "../Packet/packet.h"
//necessary header files required for network communication, file operations, threading, and other functionalities.
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>

// Reset Code =>  \033[0m
// Red Error String => \033[91m
// Green success code => \033[92m
// Blue Info code => \033[94m
// Bold Code => \033[1m

#define LOCALHOST "127.0.0.1"
#define MAX_BUFFER_SIZE 500
#define WINDOW_SIZE 5

char* fileName = "IMG_9688.MOV";
socklen_t addrLen = sizeof (struct sockaddr);

ThreadParams params;

int no_of_packets = 5;
int ackNumber = 0;

// ACK Array
int ackArray[WINDOW_SIZE];
Packet packetArray[WINDOW_SIZE]; 

//function prototypes for the functions used in the code.
void clientConstructor(clientPtr client);
void clientDestructor(clientPtr client);
void sendFile(clientPtr client);
int getConnectionSocket(clientPtr client);
void* receiveAcks(void* params);

//ClientConstructor uses the getaddrinfo function to get the server's address information based on the provided IP address and port number. 
//It then iterates over the linked list of address structures returned by getaddrinfo 
//and opens a socket using the appropriate address family, socket type, and protocol. 
//If a valid node is found, it sets the server address and breaks out of the loop
void clientConstructor(clientPtr client){
    //Server Port
    client->port = "8000";
    // Clear server address struct
    memset(&(client->hints), 0, sizeof client->hints);
    // Fill in address information
    client->hints.ai_family = AF_INET;
    client->hints.ai_socktype = SOCK_DGRAM;

    int status;

    if((status = getaddrinfo(LOCALHOST, (char*)client->port, &client->hints, &client->servinfo)) != 0){
        fprintf(stderr, "\033[91mgetaddrinfo error: %s\n\033[0m", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Find correct node in servinfo linked list
    for(struct addrinfo *ptr = client->servinfo; ptr != NULL; ptr = ptr->ai_next){
        // Open a socket
        if((client->connectionSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1){
            perror("\033[91mServer UDP socket():");
            printf("\033[0m");
            // Look for next valid node
            continue;
        }
        // If valid node found and socket() call is successful
        client->serverAddress = *ptr->ai_addr;
        break;
    }

    // if (client->serverAddress == NULL) {
	// 	fprintf(stderr, "UDP Client: Failed to create socket\n");
	// 	exit(EXIT_FAILURE);
	// }
    sendFile(client);
}
void clientDestructor(clientPtr client){
    // It frees the memory allocated for the servinfo structure using the freeaddrinfo function.
    freeaddrinfo(client->servinfo);
}
void sendFile(clientPtr client){
    pthread_t threadId;

    // Time delays for nanosleep function
    struct timespec reqTime, remTime;
    reqTime.tv_sec = 0;
    reqTime.tv_nsec = 300000000L;

    FILE* file = fopen(fileName, "rb");

    // If file is corrupt or missing
    if(file == NULL){
        perror("\033[92mError File:");
        exit(EXIT_FAILURE);
    }

    FILENAME:
	if(sendto(getConnectionSocket(client), fileName, strlen(fileName), 0, &client->serverAddress, addrLen) < 0) {
	// resend the file name
		goto FILENAME;
	}

    // Get Size of File
    struct stat fileStat;
    int fileSize = fileno(file);
	//The fstat function takes the "file descriptor fileSize obtained from the fileno function" and retrieves information about the file, including its size.
	fstat(fileSize, &fileStat);
	size_t file_size = fileStat.st_size;// file_size : variable used to store the file size
	printf("\033[92mSize of Video File: %d bytes\n\033[0m",(int) file_size);

	// sending the size of the video file to the server
	FILESIZE:
	if(sendto(getConnectionSocket(client), &file_size, sizeof(size_t), 0, &client->serverAddress, addrLen) < 0) {
	// resend the file size
		goto FILESIZE;
	}

	int data = 1;
    // While there is data
    while(data > 0){
        // 1---------make packets-------------------------
		int currentSequenceNumber = 0;
		for (int i = 0; i < no_of_packets; i++) {
            		// data = The return value of `fread` =  the number of bytes read.
			data = fread(packetArray[i].packetData, 1, MAX_BUFFER_SIZE, file);
			//sequence number and packet size are set in the `packetArray` structure for each packet
			
            		// sequence number:
			packetArray[i].packetSequenceNumber = currentSequenceNumber;
          		// packet size:
			packetArray[i].packetSize = data;
			currentSequenceNumber++;

			// last packet to be sent i.e. eof
           		if (data == 0){ 
                      		printf("\033[94mEnd of file reached.\n\033[0m");
                     		// Setting a condition for last packet
                      		packetArray[i].packetSize = -1; 
                      		// Decrementing the remaining loops 
                      		no_of_packets = i + 1; 
                      		break; 
            		}
		}

		// 2----------SEND window size packets to receiver-------------------
		for (int i = 0; i < no_of_packets; i++) {
			printf("Sending packet %d\n", packetArray[i].packetSequenceNumber);
			if(sendto(getConnectionSocket(client), &packetArray[i], sizeof(Packet), 0, &client->serverAddress, addrLen) < 0) {
				perror("UDP Client: sendto");
				exit(EXIT_FAILURE);
			}            
		}

        	//3------------ RESET the array -->  to prepare for receiving acknowledgments from the server----------
        	for (int i = 0; i < no_of_packets; i++) { 
        		ackArray[i] = 0;
        	}

		ackNumber = 0;

		//4------------- Client starts receiving ACKS--------------------------
        memset(&params,0,sizeof params);
        params.client = client;
	    
	    //new thread (ACKNOWLEDGEMENT THREAD) -> to receive acknowledgments
		pthread_create(&threadId, NULL, receiveAcks, (void*)&(params));
                   
		//The nanosleep function -> used to introduce a delay of reqTime between sending packets and checking for acknowledgments.
		nanosleep(&reqTime, &remTime);

		
		// Nested loop -> send those packets ONLY whose acks have not been received.
	    	//1.The ackArray is checked, and if the acknowledgment for a particular packet has not been received (indicated by ackArray[i] == 0), 
	    	//2.the packet is resent to the server using the sendto function.
		RESEND:
		for (int i = 0; i < no_of_packets; i++) {

			// if the ack has not been received
			if (ackArray[i] == 0) {

				// sending that packet whose ack was not received 
                		printf("Sending missing packet: %d\n",packetArray[i].packetSequenceNumber);
				if(sendto(getConnectionSocket(client), &packetArray[i], sizeof(Packet), 0, &client->serverAddress, addrLen) < 0) {
					perror("UDP Client: sendto");
					exit(1);
				}
			}
		}

		// resend the packets again whose acks have not been received
		if (ackNumber != no_of_packets) {
            	// wait for acks of the packets that were resent
            		nanosleep(&reqTime, &remTime);
			goto RESEND;
		}

		// 5 acks have been received i.e. the thread executes successfully
	    	//the thread is joined using pthread_join to synchronize the main thread with the acknowledgment thread.
		pthread_join(threadId, NULL);

		// repeat process until the eof is not reached 
    }
   // message is printed to indicate that the file transfer has completed successfully
    printf("\n\033[92mFile transfer completed successfully!\n\033[0m");
	
	close(getConnectionSocket(client)); // close the socket
	return;
}
//retrieves and returns the socket file descriptor associated with the client's connection.
int getConnectionSocket(clientPtr client){
    return client->connectionSocket;
}

//THREAD FUNCTION ->  receives acknowledgments (acks) from the server.
void* receiveAcks(void *params){
    ThreadParams *param = (ThreadParams*)params;
    clientPtr client = param->client;

    int bytesReceived = 0;
    int currentAck = 0;

    // receive 5 acks 
	for (int i = 0; i < no_of_packets; i++) {

    RECEIVE:	
    		//It receives an acknowledgment (currentAck) from the connection socket and stores the number of bytes received in the bytesReceived variable
		if((bytesReceived = recvfrom(getConnectionSocket(client), &currentAck, sizeof(currentAck), 0, (struct sockaddr*) &client->serverAddress, &addrLen)) < 0) {
			perror("\033[91mUDP Client: recvfrom\033[0m");
			exit(EXIT_FAILURE);
		} 
		
		// DUPLICATE ACK: 
		if (ackArray[currentAck] == 1) {
			// receive ack again until a unique ack is received
			goto RECEIVE; 
		}

		// UNIQUE ACK :  prints the acknowledgment value and updates the ackArray and ackNumber variables.
		printf("Ack Received: %d\n", currentAck);
		// reorder acks according to the packet's sequence number
		// make the value 1 in the acks[] array, where array position is the value of ack received (i.e. the sequence number of the packet acknowledged by the server)
		ackArray[currentAck] = 1;
		ackNumber++;

	}
	//thread function returns NULL to indicate the completion of its execution.
    return NULL;
}
