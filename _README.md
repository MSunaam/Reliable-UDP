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

This struct is used in host name lookups, and service name lookups. This struct is loaded with information and then the `getaddrinfo()` function is called.

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
