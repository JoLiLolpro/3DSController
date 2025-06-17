// 3DS Controller Server

#define VERSION 0.6

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "wireless.h"
#include "keys.h"
#include "settings.h"
#include "error.h"

int pgcd(int a, int b) {
	while (b != 0) {
	    int temp = b;
	    b = a % b;
	    a = temp;
	}
	return a;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmd, int nShow) {
	
	printf("3DS Controller Server %.1f\n", VERSION);
	
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	DWORD screenWidth = GetSystemMetrics(SM_CXSCREEN);
	DWORD screenHeight = GetSystemMetrics(SM_CYSCREEN);
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	double widthMultiplier = 0.0;
	double heightMultiplier = 0.0;

	static double smoothX = -1, smoothY = -1;
	
	static bool Debug = false; // if this is true the latency will be displayed

	LARGE_INTEGER frequency;
	LARGE_INTEGER receiveTime, applyTime;
	double latencyMs;
	static bool FirstLoop = true;
	QueryPerformanceFrequency(&frequency);

	load_settings("settings.json");

	// the Starting point of the active zone rectangle
	int StartX = StartCoor.x;
	int StartY = StartCoor.y;

	// the Ending point of the active zone rectangle
	int EndX = EndCoor.x;
	int EndY = EndCoor.y;

	int ActiveX = EndX-StartX; // size X of the active zone
	int ActiveY = EndY-StartY; // size Y of the active zone

	if (settings.Custom_Active_Zone) {
		int DS_PGCD = pgcd(ActiveX, ActiveY);
		int PC_PGCD = pgcd(screenWidth, screenHeight);

		if (ActiveX/DS_PGCD == screenWidth/PC_PGCD && ActiveY/DS_PGCD == screenHeight/PC_PGCD) {
			widthMultiplier = screenWidth / (double)ActiveX;
			heightMultiplier = screenHeight / (double)ActiveY;
		}
		else {
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
			printf("Your custom active zone doesn't match your main screen aspect ratio.\n");
			printf("The screen active zone will be the whole 3ds screen.\n");
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

			widthMultiplier = screenWidth / 320.0;
			heightMultiplier = screenHeight / 240.0;
			StartX = 0;
			StartY = 0;
		}
	}
	else {
		widthMultiplier = screenWidth / 320.0;
		heightMultiplier = screenHeight / 240.0;
		StartX = 0;
		StartY = 0;
	}


	initNetwork();

	printf("Custom_Active_Zone: %s\n", settings.Custom_Active_Zone ? "true" : "false");
	printf("Smoothing: %f\n", settings.smooth);
	printf("Port: %d\n", settings.port);
	printf("Running on: %s\n", hostName);
	printf("Your local IP(s):\n");
	printIPs();
	printf("\n");
	
	startListening();

	while (1) {
		if (receiveBuffer(sizeof(struct packet)) > 0) {
			switch (buffer.header.command) {
				case CONNECT:
					currentTouch.x = 0;
					currentTouch.y = 0;

					buffer.header.command = CONNECT;
					printf("3DS Connected!\n");

					Sleep(50); 
					sendBuffer(1, StartX, StartY, EndX, EndY);

					Sleep(50); 
					sendBuffer(1, StartX, StartY, EndX, EndY);

					Sleep(50); 
					sendBuffer(1, StartX, StartY, EndX, EndY);

					break;

				case KEYS:
					
					memcpy(&currentTouch, &buffer.Keys.touch, 4);

					if (currentTouch.x && currentTouch.y) {

                        double relativeX = (double)(currentTouch.x - StartX);
                        double relativeY = (double)(currentTouch.y - StartY);

						double targetX = relativeX * widthMultiplier;
						double targetY = relativeY * heightMultiplier;

						double alpha = settings.smooth;

						if (smoothX < 0) {
    						smoothX = targetX;
    						smoothY = targetY;
						} else {
    						smoothX = alpha * targetX + (1.0 - alpha) * smoothX;
    						smoothY = alpha * targetY + (1.0 - alpha) * smoothY;
						}

						if (Debug) {
							if (FirstLoop) {
								QueryPerformanceCounter(&receiveTime);
								FirstLoop = false;
							}
							else {
								QueryPerformanceCounter(&applyTime);
								latencyMs = (double)(applyTime.QuadPart - receiveTime.QuadPart) * 1000.0 / frequency.QuadPart;
								if (latencyMs > 30) {
									SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
									printf("Latency: %.3f ms\n", latencyMs);
									SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
								}
								else {
									printf("Latency: %.3f ms\n", latencyMs);
								}

								SetCursorPos(smoothX, smoothY);

								QueryPerformanceCounter(&receiveTime);
							}
						}
						else {
							SetCursorPos(smoothX, smoothY);
						}
					}

					
					break;
			}
		}
	}

	error("accept()");
	return 0;
}
