## Custom Structs

### Struct Client

```
typedef struct Client{
    char* port;
    int connectionSocket;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    int windowSize;
    serverAddress serverAddress;
} Client;
```

1. `port` is the port number of the client.
2. `connectionSocket` is the socket descriptor of the client.
3. `hints` is the [`struct addrinfo`](#struct-addrinfo) that is already filled out with relevant information.
4. `servinfo` is the pointer to the [`struct addrinfo`](#struct-addrinfo) linked list that is already filled out with relevant information.
5. `windowSize` is the size of the sliding window.
6. `serverAddress` is `typedef struct sockaddr serverAddress` that is used to store the server address.

### Struct Server

```
typedef struct Server {
    char* port;
    int listenSocket;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    int windowSize;
    clientAddress clientAddress;
} Server;
```

1. `port` is the port number of the server.
2. `listenSocket` is the socket descriptor of the server.
3. `hints` is the [`struct addrinfo`](#struct-addrinfo) that is already filled out with relevant information.
4. `servinfo` is the pointer to the [`struct addrinfo`](#struct-addrinfo) linked list that is already filled out with relevant information.
5. `windowSize` is the size of the sliding window.
6. `clientAddress` is `typedef struct sockaddr clientAddress` that is used to store the client address.

### Struct Packet

```
typedef struct Packet {
    int packetSequenceNumber;
    int packetSize;
    char packetData[BUFFER_SIZE];
} Packet;
```

1. `packetSequenceNumber` is the sequence number of the packet.
2. `packetSize` is the size of the packet.
3. `packetData` is the data contained in the packet.

### Struct ThreadParams

```
typedef struct ThreadParams {
    clientPtr clientOrServer;
} ThreadParams;
```

1. `clientOrServer` is the pointer to the [`struct Client`](#struct-client) or [`struct Server`](#struct-server) that is passed to the thread.

## Structs Used

### Struct Addrinfo

```
struct addrinfo {
    int ai_flags; // AI_PASSIVE, AI_CANONNAME, etc.
    int ai_family; // AF_INET, AF_INET6, AF_UNSPEC
    int ai_socktype; // SOCK_STREAM, SOCK_DGRAM
    int ai_protocol; // use 0 for "any"
    size_t ai_addrlen; // size of ai_addr in bytes
    struct sockaddr *ai_addr; // struct sockaddr_in or _in6
    char *ai_canonname; // full canonical hostname
    struct addrinfo *ai_next; // linked list, next node
};
```

This struct is used in host name lookups, and service name lookups. This struct is loaded with information and then the [`getaddrinfo()`](#getaddrinfo) function is called.

This is a linked list, as `*ai_next` points to the next element.

The pointer `ai_addr` points to the [struct sockaddr](#sockaddr). It holds socket address information for different types of sockets.

### Struct Sockaddr

```
struct sockaddr {
unsigned short sa_family; // address family, AF_xxx
    char sa_data[14]; // 14 bytes of protocol address
};
```

This struct holds address information for different types of sockets.

1. `sa_family` can be `AF_INET(IPv4)` or `AF_INET6(IPv6)`.
2. `sa_data` contains destination address and port number.

A parallel [struct `sockaddr_in`](#struct-sockaddr_in) is used for IPv4. Pointers to `struct sockaddr` can be cast to pointers of `struct sockadd_in`.

### Struct Sockaddr_In

```
struct sockaddr_in {
    short int sin_family; // Address family, AF_INET
    unsigned short int sin_port; // Port number
    struct in_addr sin_addr; // Internet address
    unsigned char sin_zero[8]; // Same size as struct sockaddr
};
```

1. `sin_zero` is used to pad this struct with zeros to make its length equal to [Struct Sockaddr](#struct-sockaddr)
2. [`struct in_addr`](#struct-in_addr) is used to store the IPv4 address.

### Struct In_Addr

```
// (IPv4 only--see struct in6_addr for IPv6)
// Internet address (a structure for historical reasons)
struct in_addr {
    uint32_t s_addr; // that's a 32-bit int (4 bytes)
};
```

### Struct Timespec

```
struct timespec {
    time_t tv_sec; // seconds
    long tv_nsec; // nanoseconds
};
```

This struct is used to store the time in seconds and nanoseconds.

1. `tv_sec` is the number of seconds.
2. `tv_nsec` is the number of nanoseconds.

## Functions Used

### GetAddrInfo()

`getaddrinfo()` helps set up the structs you need later on. This function does DNS Lookups and service name lookups and fills all the necessary structs.

```
int getaddrinfo(
    const char *node, // e.g. "www.example.com" or IP
    const char *service, // e.g. "http" or port number const struct addrinfo *hints,
    struct addrinfo **res
    );
```

This gives a linked list of [`struct addrinfo`](#struct-addrinfo), res, of results.

1. The Node parameter is the hostname or IP Address to connect to.
2. The Service parameter is a port number or the name of a service like "ftp", "telnet", or "smtp" etc.
3. The Hint parameter points to a [`struct addrinfo`](#struct-addrinfo) that is already filled out with relevant information.

**freeaddrinfo()** is used to free up the linked list `res`.

### Gai_Strerror()

```
const char *gai_strerror(int errcode);
```

`gai_strerror()` returns a string describing the error value passed in the argument errcode.

### Socket()

```
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

`socket()` simply returns to you a socket descriptor that you can use in later system calls, or -1 on error. The global variable errno is set to the error’s value.

1. Domain is `PF_INET` (IPv4) or `PF_INET6`(IPv6).
2. Type is `SOCK_STREAM`(TCP) or `SOCK_DGRAM`(UDP).
3. Protocol can be set by `getprotobyname('tcp')` or `getprotobyname('udp')`.

### Bind()

```
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
```

`bind()` is used to bind a socket to an address and port number on which the server will run. It returns 0 on success and -1 on failure.

1. `sockfd` is the socket descriptor returned by [`socket()`](#socket).
2. `my_addr` pointer to [`sockaddr` struct](#struct-sockaddr) is used to store IP Address, and port number.
3. `addrlen` is the length of `myaddr`.

### Sendto()

```
#include <sys/types.h>
#include <sys/socket.h>

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
```

`sendto()` is used to send data to a specific destination. It returns the number of bytes sent.

1. `sockfd` is the socket descriptor returned by [`socket()`](#socket).
2. `buf` is the buffer containing the data to be sent.
3. `len` is the length of the buffer.
4. `flags` is the type of message transmission.
5. `dest_addr` is the pointer to [`sockaddr` struct](#struct-sockaddr) containing the destination IP Address and port number.
6. `addrlen` is the length of `dest_addr`.

### Recvfrom()

```
#include <sys/types.h>
#include <sys/socket.h>

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
```

`recvfrom()` is used to receive data from a specific source. It returns the number of bytes received.

1. `sockfd` is the socket descriptor returned by [`socket()`](#socket).
2. `buf` is the buffer to store the received data.
3. `len` is the length of the buffer.
4. `flags` is the type of message transmission.
5. `src_addr` is the pointer to [`sockaddr` struct](#struct-sockaddr) containing the source IP Address and port number.
6. `addrlen` is the length of `src_addr`.

### Memset()

```
void *memset(void *s, int c, size_t n);
```

`memset()` is used to fill a block of memory with a particular value. It returns a pointer to the memory area `s`.

1. `s` is the pointer to the block of memory to fill.
2. `c` is the value to be set.
3. `n` is the number of bytes to be set to the value.

### fprintf()

```
int fprintf(FILE *stream, const char *format, ...);
```

`fprintf()` is used to print formatted output to a stream. It returns the number of characters printed.

1. `stream` is the pointer to a file stream where the output is to be printed.
2. `format` is the format string.
3. `...` is the variable number of arguments to be printed according to the format string.

### Perror()

```
void perror(const char *s);
```

`perror()` prints a descriptive error message to stderr. It takes the string s as an argument and prints it followed by a colon and then prints the error message corresponding to the current value of errno.

### Close()

```
#include <unistd.h>

int close(int fd);
```

`close()` closes a file descriptor, so that it no longer refers to any file and may be reused. It returns zero on success and -1 on failure.

1. `fd` is the file descriptor to be closed.

### Inet_ntop()

```
#include <arpa/inet.h>

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
```

`inet_ntop()` converts the network address structure src in the af address family into a character string. The resulting string is copied to the buffer pointed to by dst, which must be a non-null pointer. The caller specifies the number of bytes available in this buffer in the argument size.

1. `af` is the address family.
2. `src` is the pointer to the network address structure.
3. `dst` is the pointer to the buffer where the string is to be stored.
4. `size` is the size of the buffer.

### Ntohs()

```
#include <arpa/inet.h>

uint16_t ntohs(uint16_t netshort);
```

`ntohs()` converts the unsigned short integer netshort from network byte order to host byte order.
It returns the value in host byte order.

1. `netshort` is the unsigned short integer to be converted.

### Fopen()

```
#include <stdio.h>

FILE *fopen(const char *path, const char *mode);
```

`fopen()` opens the file whose name is the string pointed to by path and associates a stream with it. It returns a pointer to the stream on success and NULL on failure.

1. `path` is the path to the file to be opened.
2. `mode` is the mode in which the file is to be opened.

### Fclose()

```
#include <stdio.h>

int fclose(FILE *stream);
```

`fclose()` closes the stream. It returns zero on success and EOF on failure.

1. `stream` is the pointer to the stream to be closed.

### Pthread_create()

```
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
```

`pthread_create()` creates a new thread, with attributes specified by attr, within a process. If attr is NULL, then the thread is created with default attributes. It returns zero on success and an error number on failure.

1. `thread` is the pointer to the thread to be created.
2. `attr` is the pointer to the thread attributes.
3. `start_routine` is the pointer to the function to be executed by the thread.
4. `arg` is the pointer to the argument to be passed to the function.

### Pthread_join()

```
#include <pthread.h>

int pthread_join(pthread_t thread, void **retval);
```

`pthread_join()` waits for the thread specified by thread to terminate. If that thread has already terminated, then pthread_join() returns immediately. The thread specified by thread must be joinable. It returns zero on success and an error number on failure.

1. `thread` is the thread to be joined.
2. `retval` is the pointer to the location where the exit status of the thread is to be stored.

### NanoSleep()

```
#include <time.h>

int nanosleep(const struct timespec *req, struct timespec *rem);
```

`nanosleep()` suspends the execution of the calling thread until either at least the time specified in \*req has elapsed, or the delivery of a signal that triggers the invocation of a handler in the calling thread or that terminates the process. It returns zero on success and -1 on failure.

1. `req` is the pointer to the struct timespec that specifies the interval of time to suspend execution.
2. `rem` is the pointer to the struct timespec that stores the remaining time if the call is interrupted by a signal handler.

### Fileno()

```
#include <stdio.h>

int fileno(FILE *stream);
```

`fileno()` returns the integer file descriptor associated with the stream pointed to by stream.

1. `stream` is the pointer to the stream.

### Fstat()

```
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int fstat(int fd, struct stat *buf);
```

`fstat()` returns information about the file associated with the file descriptor fd in the buffer pointed to by buf. It returns zero on success and -1 on failure.

1. `fd` is the file descriptor.
2. `buf` is the pointer to the buffer where the information is to be stored.

### Fread()

```
#include <stdio.h>

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
```

`fread()` reads nmemb items of data, each size bytes long, from the stream pointed to by stream, storing them at the location given by ptr. It returns the number of items successfully read.

1. `ptr` is the pointer to the buffer where the data is to be stored.
2. `size` is the size of each item to be read.
3. `nmemb` is the number of items to be read.
4. `stream` is the pointer to the stream.

### Fwrite()

```
#include <stdio.h>

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
```

`fwrite()` writes nmemb items of data, each size bytes long, to the stream pointed to by stream, obtaining them from the location given by ptr. It returns the number of items successfully written.

1. `ptr` is the pointer to the buffer where the data is to be read from.
2. `size` is the size of each item to be written.
3. `nmemb` is the number of items to be written.
4. `stream` is the pointer to the stream.
