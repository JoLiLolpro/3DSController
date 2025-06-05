#pragma once

#include <string.h>

#include <3ds.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>


#define DEFAULT_PORT 8889

enum NET_COMMANDS {
	CONNECT,
	KEYS
};

// It is deliberately set up to have an anonymous struct as well as a named struct for convenience, not a mistake!
struct packet {
    struct packetHeader {
        unsigned char command;
    } header;

    union {
        struct connectPacket {
        } Connect;

        struct keysPacket {
            unsigned int keys;
            struct {
                unsigned short x;
                unsigned short y;
            } touch;
        } Keys;
    };
};


extern int sock;
extern struct sockaddr_in sain, saout;
extern struct packet outBuf, rcvBuf;

extern socklen_t sockaddr_in_sizePtr;

bool openSocket(int port);
void sendBuf(int length);
int receiveBuffer(int length);
void sendConnectionRequest(void);
void sendKeys(unsigned int keys, touchPosition touch);
