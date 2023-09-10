#include "socket.h"
#include "log.h"

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#define SOCKET_LOG_FATAL(msg) log_fatal(msg ": %d\n", WSAGetLastError());

#elif __linux__

#include <errno.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_LOG_FATAL(msg) log_fatal(msg ": %s\n", strerror(errno));

#endif

int socket_init(socket_t *s, const char *host, uint16_t port) {
    int result;

#ifdef _WIN32
    WSADATA wsaData;
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        SOCKET_LOG_FATAL("WSAStartup failed");
        return 1;
    }
#endif

    socket_t f = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (f == -1) {
        SOCKET_LOG_FATAL("Create socket failed");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    int opt_val = 0;
    setsockopt(f, IPPROTO_IPV6, IPV6_V6ONLY, &opt_val, sizeof(opt_val));
#ifdef _WIN32
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(f, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
#endif

    address_t addr;
    result = resolve_address(&addr, host, port);
    if (result != 0) {
        log_fatal("Fail to resolve host address\n");
        socket_close(f);
        return 1;
    }

    result = bind(f, (struct sockaddr *)&addr, sizeof(addr));
    if (result != 0) {
        SOCKET_LOG_FATAL("Bind socket failed");
        socket_close(f);
        return 1;
    }
    *s = f;
    return 0;
}

void socket_close(socket_t s) {
#ifdef _WIN32
    closesocket(s);
    WSACleanup();
#elif __linux__
    close(s);
#endif
}

int socket_recv(socket_t s, address_t *from, void *buffer, int maxlength, int *len) {
    int result;
    socklen_t addrSize = sizeof(address_t);
    result = recvfrom(s, buffer, maxlength, 0, (struct sockaddr *)from, &addrSize);
    if (result == -1) {
        SOCKET_LOG_FATAL("recvfrom failed");
        socket_close(s);
        return 1;
    }
    *len = result;
    return 0;
}

int socket_send(socket_t s, const address_t *to, const void *buffer, int len) {
    int result;
    result = sendto(s, buffer, len, 0, (struct sockaddr *)to, sizeof(address_t));
    if (result == -1) {
        SOCKET_LOG_FATAL("sendto failed");
        socket_close(s);
        return 1;
    }
    return 0;
}

int resolve_address(address_t *addr, const char *host, uint16_t port) {
    memset(addr, 0, sizeof(address_t));
    addr->sin6_family = AF_INET6;
    int result = inet_pton(AF_INET6, host, &addr->sin6_addr);
    if (result == 0) {
        memset(&addr->sin6_addr, 0, sizeof(addr->sin6_addr));
        struct in_addr addr4;
        result = inet_pton(AF_INET, host, &addr4);
        if (result == 0) return 1;
        addr->sin6_addr.s6_addr[10] = 0xff;
        addr->sin6_addr.s6_addr[11] = 0xff;
        memcpy(&addr->sin6_addr.s6_addr[12], &addr4.s_addr, 4);
    }
    addr->sin6_port = htons(port);
    return 0;
}

char *address_host(address_t addr) {
    static char buf[100];
    if (IN6_IS_ADDR_V4MAPPED(&addr.sin6_addr)) {
        inet_ntop(AF_INET, (char *)&addr.sin6_addr + 12, (void *)&buf, sizeof(buf));
    } else {
        inet_ntop(AF_INET6, &addr.sin6_addr, (void *)&buf, sizeof(buf));
    }
    return buf;
}
