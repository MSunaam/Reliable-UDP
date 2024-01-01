# Reliable Video File Transfer with UDP
This code is for .mp4 video files.
The input video file should have name input_video.mp4
The output file will be generated as output_video.mp4

## Description

### Idea:
This project presents the design and implementation of a reliable video file transfer system using User Datagram Protocol (UDP) and C sockets. The project aims to overcome network unreliability by incorporating essential mechanisms such as sequence numbers, retransmission, window size, and reordering on the receiver side. This report provides an overview of UDP protocol, explains the implementation of reliability using UDP, and discusses the structure and functionality of the developed client and server applications.

## UDP and TCP Overview

### TCP:
TCP is a connection-oriented protocol that establishes a reliable, ordered, and error-checked connection between two applications. It provides features such as flow control, congestion control, and error recovery mechanisms using acknowledgments and retransmissions.
### UDP:
UDP is a lightweight transport layer protocol that operates on top of IP (Internet Protocol). It offers a simple, connectionless communication model without the overhead of establishing and maintaining a connection. UDP provides fast and efficient data transmission but does not guarantee reliability, ordering, or congestion control.
## Implementing Reliability in UDP:
To make UDP reliable, several mechanisms are implemented in the project:

### 	Sequence Numbers
Sequence numbers are assigned to each packet to maintain the order of transmission. Both the sender and receiver keep track of the sequence numbers to ensure packets are processed in the correct order.

### 	Retransmission (Selective Repeat)
Selective Repeat is a retransmission technique used to handle lost or corrupted packets. The sender maintains a window of packets waiting for acknowledgment. 

### 	Window Size
A sliding window mechanism is implemented with a window size of 5 UDP segments. This mechanism allows the sender to transmit multiple packets before waiting for acknowledgments, improving overall efficiency.

### 	Reordering on the Receiver Side
The receiver reorders the received packets based on their sequence numbers before writing them to a file. This ensures that the video file is reconstructed correctly.

## Code Structure
![Structure](https://github.com/MSunaam/Reliable-UDP/blob/main/Resources/structure.png)
## Functionality and Implementation
### Client:
Following functionalities are implemented by client:
* The sender reads the file specified by the filename provided.
*	The sender divides the file data into packets of 500 bytes each.
*	It sends the packets over the UDP connection to the receiver.
*	If a packet fails to reach the receiver, the sender performs retransmission for that packet.
*	The sender sends packets in a window size of 5 UDP segments.
*	It waits for acknowledgments from the receiver before sending the next window of packets.
*	The wait time in-between sending each packet is 0.03 seconds.
*	The sender binds to a listen port to receive acknowledgments and other signaling from the receiver.
*	It waits for acknowledgments from the receiver for each sent packet.
*	The code creates a separate thread using the pthread_create function. 
*	receiveAcks function is used as the start routine for the thread. 
*	The receiveAcks function is responsible for receiving acknowledgments from the server for the sent packets. It runs in a separate thread, concurrently with the main thread that continues sending packets.
*	The main purpose of this thread is to listen for acknowledgments and update the acknowledgment status in an array called ackArray.
*	Within the receiveAcks function, a loop is used to continuously receive acknowledgments from the server until all packets have been acknowledged.
*	The received acknowledgment contains the sequence number of the acknowledged packet. The thread updates the corresponding entry in the ackArray to indicate that the packet has been successfully received by the server.
*	After successfully transferring the entire file, the sender completes the transfer.
*	It terminates and exits the application

### Server:
Following functionalities are implemented by Server:

*	Server creates a socket and binds it to the specified port to listen for incoming connections.
*	The server enters a loop to receive the file from the client. Within the loop, it performs the following steps:
*	It waits to receive the file name, file size, and the number of packets from the client.
*	It creates a file with the received file name to write the received data.
*	It initializes the packet array, acknowledgment array, number of received acknowledgments, current sequence number, and the total number of packets.
*	The receiveFile function creates a new thread using pthread_create and passes the receivePackets function as the thread routine. 
*	The receivePackets function is the entry point for the receive packets thread. It receives packets and updates the packetArray and ackArray data structures.
*	Each thread receives packets until the specified number of packets is received or a duplicate packet is received.
*	Upon receiving a packet, it updates the packet array and acknowledgment array accordingly.
*	It sends an acknowledgment for each received packet.
*	Once all packets are received, it writes the received data to the file.
*	Inside the receiveFile function, after creating the receive packets thread, there is a delay using nanosleep to allow time for packets to start arriving before sending acknowledgments.
*	The main thread then waits for the receive packets thread to complete using pthread_join. This ensures that all packets have been received before processing them.
*	After successfully receiving and writing the entire file, the server completes the file transfer.


## Design:
### Client:
![Client](https://github.com/MSunaam/Reliable-UDP/blob/main/Design/Client.png)
### Server:
![Server](https://github.com/MSunaam/Reliable-UDP/blob/main/Design/Server.png)

## Output

### Client:
![Client Output 1](https://github.com/MSunaam/Reliable-UDP/blob/main/Resources/client1.png)

![Client Output 2](https://github.com/MSunaam/Reliable-UDP/blob/main/Resources/client2.png)
### Server:
![Server Output 1](https://github.com/MSunaam/Reliable-UDP/blob/main/Resources/server1.png)

![Server Output 2](https://github.com/MSunaam/Reliable-UDP/blob/main/Resources/server2.png)
