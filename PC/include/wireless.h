#pragma once

#ifndef WINVER
	#define WINVER 0x0500
#endif

#include <windows.h>
#include <winsock.h>

#include <stddef.h>

#define SCREENSHOT_CHUNK 4000

#define IP INADDR_ANY

enum NET_COMMANDS {
	CONNECT,
	KEYS,
};


struct packet {
    struct packetHeader {
        unsigned char command;
    } header;

    union {
        struct connectPacket {
            struct {
                unsigned short x;
                unsigned short y;
            } activeZoneStart;
            struct {
                unsigned short x;
                unsigned short y;
            } activeZoneEnd;
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

extern SOCKET listener;
extern SOCKET client;

extern struct sockaddr_in client_in;

extern int sockaddr_in_sizePtr;

extern struct packet buffer;
extern char hostName[80];

void initNetwork(void);
void printIPs(void);
void startListening(void);
void sendBuffer(int length, int XS, int YS, int XE, int YE);
int receiveBuffer(int length);
