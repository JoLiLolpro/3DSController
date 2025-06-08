// 3DS Controller Server

#define VERSION 0.6

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "wireless.h"
#include "keys.h"
#include "settings.h"
#include "error.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmd, int nShow) {
	
	printf("3DS Controller Server %.1f\n", VERSION);
	
	DWORD screenWidth = GetSystemMetrics(SM_CXSCREEN);
	DWORD screenHeight = GetSystemMetrics(SM_CYSCREEN);
	
	double widthMultiplier = screenWidth / 160.0;   // only take the top-left part of the touchscreen
	double heightMultiplier = screenHeight / 120.0; // only take the top-left part of the touchscreen

	static double smoothX = -1, smoothY = -1;
	bool Touched = false;
	
	if(!readSettings()) {
		printf("Couldn't read settings file, using default key bindings.\n");
	}
	
	initNetwork();
	
	printf("Port: %d\n", settings.port);
	printf("Running on: %s\n", hostName);
	printf("Your local IP(s):\n");
	printIPs();
	printf("\n");
	
	startListening();

	while (1) {
		while (receiveBuffer(sizeof(struct packet)) > 0) {
			switch (buffer.header.command) {
				case CONNECT:
					lastTouch.x = 0;
					lastTouch.y = 0;
					currentTouch.x = 0;
					currentTouch.y = 0;

					buffer.header.command = CONNECT;
					printf("3DS Connected!\n");

					Sleep(50); 
					sendBuffer(1);

					Sleep(50); 
					sendBuffer(1);

					Sleep(50); 
					sendBuffer(1);

					break;

				case KEYS:
					memcpy(&currentTouch, &buffer.Keys.touch, 4);

					if (currentTouch.x && currentTouch.y) {
						double targetX = (double)(currentTouch.x - 160) * widthMultiplier;
						double targetY = (double)(currentTouch.y - 120) * heightMultiplier;

						double alpha = settings.smooth;
						if (smoothX < 0) {
    						smoothX = targetX;
    						smoothY = targetY;
						} else {
    						smoothX = alpha * targetX + (1.0 - alpha) * smoothX;
    						smoothY = alpha * targetY + (1.0 - alpha) * smoothY;
						}
						Touched = true;
					}
					
					break;
			}
		}

		if (Touched) {
			SetCursorPos(smoothX, smoothY);
			Touched = false;
		}
	}

	error("accept()");
	return 0;
}
