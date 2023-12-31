#pragma once
// Buffer Size is 500 as per problem statement
#define BUFFER_SIZE 500

typedef struct Packet {
    int packetSequenceNumber;
    int packetSize;
    char packetData[BUFFER_SIZE];
} Packet;

typedef Packet* packetPtr;