#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>

typedef struct sockaddr serverAddress;
typedef struct Client{
    char* port;
    int connectionSocket;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    int windowSize;
    serverAddress serverAddress;
} Client;
typedef Client* clientPtr; 

typedef struct ThreadParams {
    clientPtr client;
} ThreadParams;

void clientConstructor(clientPtr client);
void clientDestructor(clientPtr client);
void sendFile(clientPtr client);
void* receiveAcks(void* params);
int getConnectionSocket(clientPtr client);

