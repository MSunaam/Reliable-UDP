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
This struct holds address information for different types of sockets.1. `sa_family` can be `AF_INET(IPv4)` or `AF_INET6(IPv6)`.
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

### Socket()
```
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```
`socket()` simply returns to you a socket descriptor that you can use in later system calls, or -1 on error. The global variable errno is set to the error’s value.

1. Domain is `PF_INET` (IPv4) or `PF_INET6`(IPv6).
2. Type is `SOCK_STREAM`(TCP) or `SOCK_DGRAM`(UDP).
3. Protocol can be set by `getprotobyname('tcp)` or `getprotobyname('udp)`.

### Bind()
```
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
```

1. `sockfd` is the socket descriptor returned by [`socket()`](#socket).
2. `my_addr` pointer to [`sockaddr` struct](#struct-sockaddr) is used to store IP Address, and port number.
3. `addrlen` is the length of `myaddr`.