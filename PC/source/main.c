// 3DS Mouse Server

#define VERSION 1.7

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

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	load_settings("settings.json");

	// setting variables for the latency measure (debug)

	LARGE_INTEGER frequency;
	LARGE_INTEGER receiveTime, applyTime;
	double latencyMs;
	bool FirstLoop = true;
	QueryPerformanceFrequency(&frequency);

	// define global variables

	bool touching = false;

	double widthMultiplier = 0.0;
	double heightMultiplier = 0.0;

	int smoothX = -1, smoothY = -1;
	double offset = settings.smooth;

	bool connected = false;

	// the Starting point of the active zone rectangle for 3ds
	int StartX = StartCoor.x;
	int StartY = StartCoor.y;

	// the Ending point of the active zone rectangle for 3ds
	int EndX = EndCoor.x;
	int EndY = EndCoor.y;

	int ActiveX = EndX-StartX; // size X of the active zone for 3ds
	int ActiveY = EndY-StartY; // size Y of the active zone for 3ds

	// same but for the PC screen
	int ScreenStartX = ScreenStartCoor.x;
	int ScreenStartY = ScreenStartCoor.y;
	int ScreenEndX = ScreenEndCoor.x;
	int ScreenEndY = ScreenEndCoor.y;
	int ScreenActiveX = ScreenEndX-ScreenStartX;
	int ScreenActiveY = ScreenEndY-ScreenStartY;

	// map the 3DS screen to your screen (May stretch the active zone depending on aspect ratio)

	if (settings.Custom_Active_Zone) {
		widthMultiplier = ScreenActiveX / ActiveX;
		heightMultiplier = ScreenActiveY / ActiveY;
	} else {
		widthMultiplier = ScreenActiveX / 320.0;
		heightMultiplier = ScreenActiveY / 240.0;
		StartX = 0;
		StartY = 0;
		EndX = 320;
		EndY = 240;
	}

	initNetwork();

	printf("Custom_Active_Zone: %s\n", settings.Custom_Active_Zone ? "true" : "false");
	printf("Drawing: %s\n", settings.TapFeature ? "true" : "false");
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

				// adjust the touch position to your screen
				int targetX = relativeX * widthMultiplier + ScreenStartX;
				int targetY = relativeY * heightMultiplier + ScreenStartY;

				// smooth the movement based on an offset between 0 and 1
				if (smoothX < 0 || !touching) {
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

						INPUT move = {0};
            			move.type = INPUT_MOUSE;
            			move.mi.dx = smoothX;
            			move.mi.dy = smoothY;
            			move.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

            			SendInput(1, &move, sizeof(move)); // set the mouse position

						if (settings.TapFeature && !touching) {
							INPUT down = {0};
							down.type = INPUT_MOUSE;
							down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

							SendInput(1, &down, sizeof(INPUT)); // simulate the click hold
						}
						if (!touching) touching = true;
						
						QueryPerformanceCounter(&receiveTime);
					}
				} else {
					
					INPUT move = {0};
            		move.type = INPUT_MOUSE;
            		move.mi.dx = smoothX;
            		move.mi.dy = smoothY;
            		move.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

            		SendInput(1, &move, sizeof(move)); // set the mouse position

					if (settings.TapFeature && !touching) {
						INPUT down = {0};
						down.type = INPUT_MOUSE;
						down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

						SendInput(1, &down, sizeof(INPUT)); // simulate the click hold
					}
					if (!touching) touching = true;
				}
			} else {
				if (settings.TapFeature && touching) {
					INPUT up = {0};
					up.type = INPUT_MOUSE;
					up.mi.dwFlags = MOUSEEVENTF_LEFTUP; // release the click if no more touch info are sent

					SendInput(1, &up, sizeof(INPUT));
				}
				if (touching) touching = false;
				
			}
		}
	}
	return 0;
}
