#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include "error.h"
#include "settings.h"
#include "wireless.h"

int listener;
int client;

struct sockaddr_in client_in;
socklen_t sockaddr_in_sizePtr = sizeof(struct sockaddr_in);

struct packet buffer;
char hostName[80];

void initNetwork(void) {
    if(gethostname(hostName, sizeof(hostName)) < 0) {
        error("gethostname()");
    }
}


void printIPs() {
    struct ifaddrs *ifaddr, *ifa;
    int family;
    char addr_str[INET6_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            void *addr_ptr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;

            if (inet_ntop(family, addr_ptr, addr_str, sizeof(addr_str)) == NULL) {
                perror("inet_ntop");
                continue;
            }

            if (strcmp(ifa->ifa_name, "lo") != 0) {
                printf("  %s: %s\n", ifa->ifa_name, addr_str);
            }
        }
    }

    freeifaddrs(ifaddr);
}

void startListening(void) {
    listener = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(listener < 0) {
        error("socket()");
    }

    struct sockaddr_in serverInfo;
    memset(&serverInfo, 0, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = htonl(INADDR_ANY);
    serverInfo.sin_port = htons(settings.port);

    int flags = fcntl(listener, F_GETFL, 0);
    if(flags < 0 || fcntl(listener, F_SETFL, flags | O_NONBLOCK) < 0) {
        error("fcntl()");
    }

    if(bind(listener, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) < 0) {
        error("bind()");
    }
}

void sendBuffer(int length, int XS, int YS, int XE, int YE) {
    buffer.Connect.activeZoneStart.x = XS;
    buffer.Connect.activeZoneStart.y = YS;
    buffer.Connect.activeZoneEnd.x = XE;
    buffer.Connect.activeZoneEnd.y = YE;

    length = offsetof(struct packet, Keys) + sizeof(struct connectPacket);

    ssize_t sent = sendto(listener, &buffer, length, 0, (struct sockaddr *)&client_in, sizeof(client_in));

    if(sent != length) {
        error("sendto");
    }
}

int receiveBuffer(int length) {
    socklen_t sockaddr_in_sizePtr = sizeof(client_in);
    int bytesReceived = recvfrom(listener, &buffer, length, 0, (struct sockaddr *)&client_in, &sockaddr_in_sizePtr);
    return bytesReceived;
}
