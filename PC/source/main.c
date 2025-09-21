// 3DS Mouse Server

#define VERSION 1.6

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <timeapi.h>


#include "wireless.h"
#include "settings.h"
#include "error.h"


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmd, int nShow) {
	
	printf("Welcome to 3DS Mouse, version %.1f\n", VERSION);

	// set higher priority in Windows (Unsure if there's a big performance gain, but kept it just in case)

	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	timeBeginPeriod(1);

	// get system infos and load the settings file

	DWORD screenWidth = GetSystemMetrics(SM_CXSCREEN);
	DWORD screenHeight = GetSystemMetrics(SM_CYSCREEN);
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	load_settings("settings.json");

	// setting variables for the latency measure (debug)

	LARGE_INTEGER frequency;
	LARGE_INTEGER receiveTime, applyTime;
	double latencyMs;
	bool FirstLoop = true;
	QueryPerformanceFrequency(&frequency);

	// define global variables

	double widthMultiplier = 0.0;
	double heightMultiplier = 0.0;

	int smoothX = -1, smoothY = -1;
	double offset = settings.smooth;

	bool connected = false;

	// the Starting point of the active zone rectangle
	int StartX = StartCoor.x;
	int StartY = StartCoor.y;

	// the Ending point of the active zone rectangle
	int EndX = EndCoor.x;
	int EndY = EndCoor.y;

	int ActiveX = EndX-StartX; // size X of the active zone
	int ActiveY = EndY-StartY; // size Y of the active zone

	// map the 3DS screen to your monitor resolution (May stretch the active zone depending on aspect ratio)

	if (settings.Custom_Active_Zone) {
		widthMultiplier = screenWidth / ActiveX;
		heightMultiplier = screenHeight / ActiveY;
	} else {
		widthMultiplier = screenWidth / 320.0;
		heightMultiplier = screenHeight / 240.0;
		StartX = 0;
		StartY = 0;
	}

	initNetwork();

	printf("Custom_Active_Zone: %s\n", settings.Custom_Active_Zone ? "true" : "false");
	printf("Debug: %s\n", settings.debug ? "true" : "false");
	printf("Smoothing: %f\n", settings.smooth);
	printf("Port: %d\n", settings.port);
	printf("Your local IP(s):\n");
	printIPs(); // print the ipv4 of all your adapters
	printf("\n");
	
	startListening(); // init the network server

	printf("Waiting for 3DS...\n");

	// first loop that wait for a 3DS connection

	while (!connected) {
		if (receiveBuffer(sizeof(struct packet)) > 0 && buffer.header.command == CONNECT) {
			// send back a buffer with the custom active zone so the 3DS can draw the blue lines

			sendConnectBuffer(1, StartX, StartY, EndX, EndY);

			Sleep(50);
			sendConnectBuffer(1, StartX, StartY, EndX, EndY);

			Sleep(50);
			sendConnectBuffer(1, StartX, StartY, EndX, EndY);

			printf("3DS Connected!\n");
			connected = true;
		}
	}

	// main loop that update the mouse position

	while (true) {
		if (receiveBuffer(sizeof(struct packet)) > 0 && buffer.header.command == KEYS) {

			memcpy(&currentTouch, &buffer.Keys.touch, sizeof(currentTouch)); // put in memory the touch values that the 3DS gave us

			if (currentTouch.x && currentTouch.y) { // make sure its not putting the cursor at the top left when 3ds stop sending packet

				// Calculate position relative to the active zone's origin
                int relativeX = (currentTouch.x - StartX);
                int relativeY = (currentTouch.y - StartY);

				// adjust the touch position to your screen resolution
				int targetX = relativeX * widthMultiplier;
				int targetY = relativeY * heightMultiplier;

				// smooth the movement based on an offset between 0 and 1
				if (smoothX < 0) {
    				smoothX = targetX;
    				smoothY = targetY;
				} else {
    				smoothX = offset * targetX + (1.0 - offset) * smoothX;
    				smoothY = offset * targetY + (1.0 - offset) * smoothY;
				}

				if (settings.debug) {
					if (FirstLoop) {
						QueryPerformanceCounter(&receiveTime);
						FirstLoop = false;
					} else {
						QueryPerformanceCounter(&applyTime);
						latencyMs = (double)(applyTime.QuadPart - receiveTime.QuadPart) * 1000.0 / frequency.QuadPart;
						if (latencyMs > 30) {
							SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY); // set the console color to red
							printf("Latency: %.3f ms\n", latencyMs);
							SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // set the console color back to normal
						} else {
							printf("Latency: %.3f ms\n", latencyMs);
						}

						SetCursorPos(smoothX, smoothY); // set the mouse position
						QueryPerformanceCounter(&receiveTime);
					}
				} else {
					SetCursorPos(smoothX, smoothY); // set the mouse position
				}
			}
		}
	}
	return 0;
}
