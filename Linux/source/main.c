// 3DS Controller Server

#define VERSION 1.6

#define RED_TEXT     "\x1b[31m"
#define NORMAL_TEXT   "\x1b[0m"


#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <X11/Xlib.h>

#include "wireless.h"
#include "settings.h"
#include "error.h"

int uinput_fd = -1;

void init_uinput_device(int screen_Width, int screen_Height) {
    uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinput_fd < 0) {
        perror("open /dev/uinput");
        return;
    }

    ioctl(uinput_fd, UI_SET_EVBIT, EV_ABS);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_X);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TOUCH);
    ioctl(uinput_fd, UI_SET_EVBIT, EV_SYN);

    struct uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "3ds-tablet");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1234;
    uidev.id.product = 0x5678;
    uidev.id.version = 1;

    uidev.absmin[ABS_X] = 0;
    uidev.absmax[ABS_X] = screen_Width;
    uidev.absmin[ABS_Y] = 0;
    uidev.absmax[ABS_Y] = screen_Height;

    write(uinput_fd, &uidev, sizeof(uidev));
    ioctl(uinput_fd, UI_DEV_CREATE);
    usleep(100000);
}


void set_cursor_pos(int absX, int absY, bool touching) {
    struct input_event ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = EV_ABS; ev.code = ABS_X; ev.value = absX;
    write(uinput_fd, &ev, sizeof(ev));

    memset(&ev, 0, sizeof(ev));
    ev.type = EV_ABS; ev.code = ABS_Y; ev.value = absY;
    write(uinput_fd, &ev, sizeof(ev));

    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY; ev.code = BTN_TOUCH; ev.value = touching ? 1 : 0;
    write(uinput_fd, &ev, sizeof(ev));

    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN; ev.code = SYN_REPORT; ev.value = 0;
    write(uinput_fd, &ev, sizeof(ev));
}


int main(int argc, char *argv[]) {
	
	printf("Welcome to 3DS Mouse, version %.1f\n", VERSION);

	// get system infos and load the settings file

	Display *display = XOpenDisplay(NULL);
	if (!display) {
    	fprintf(stderr, RED_TEXT "Error: Could not open X display.\n" NORMAL_TEXT);
    	return 1;
	}

	Screen *s = DefaultScreenOfDisplay(display);
	unsigned int screenWidth = s->width;
	unsigned int screenHeight = s->height;

	load_settings("settings.json");

	// setting variables for the latency measure (debug)

	struct timespec receiveTime, applyTime;
	double latencyMs;
	static bool FirstLoop = true;

	// define global variables

	double widthMultiplier = 0.0;
	double heightMultiplier = 0.0;

	static double smoothX = -1, smoothY = -1;
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
		widthMultiplier = screenWidth / (double)ActiveX;
		heightMultiplier = screenHeight / (double)ActiveY;
	}
	else {
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

			usleep(50000);
			sendConnectBuffer(1, StartX, StartY, EndX, EndY);

			usleep(50000);
			sendConnectBuffer(1, StartX, StartY, EndX, EndY);

			printf("3DS Connected!\n");
			connected = true;
		}
	}

	// main loop that update the mouse position

	while (true) {
		if (receiveBuffer(sizeof(struct packet)) > 0 && buffer.header.command == KEYS) {

			memcpy(&currentTouch, &buffer.Keys.touch, 4); // put in memory the touch values that the 3DS gave us

			if (currentTouch.x && currentTouch.y) { // make sure its not putting the cursor at the top left when 3ds stop sending packet

				// Calculate position relative to the active zone's origin
                double relativeX = (double)(currentTouch.x - StartX);
                double relativeY = (double)(currentTouch.y - StartY);

				// adjust the touch position to your screen resolution
				double targetX = relativeX * widthMultiplier;
				double targetY = relativeY * heightMultiplier;

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
						clock_gettime(CLOCK_MONOTONIC, &receiveTime);
						init_uinput_device(screenWidth, screenHeight);
						FirstLoop = false;
					}
					else {
						clock_gettime(CLOCK_MONOTONIC, &applyTime);
						latencyMs = (applyTime.tv_sec - receiveTime.tv_sec) * 1000.0 + (applyTime.tv_nsec - receiveTime.tv_nsec) / 1000000.0;
						
						if (latencyMs > 30) {
							printf(RED_TEXT "Latency: %.3f ms\n" NORMAL_TEXT, latencyMs); // print in red
						}
						else {
							printf("Latency: %.3f ms\n", latencyMs);
						}

						set_cursor_pos(smoothX, smoothY, 1); // set the mouse position

						clock_gettime(CLOCK_MONOTONIC, &receiveTime);
					}
				}
				else {
					if (FirstLoop) {
						init_uinput_device(screenWidth, screenHeight); // init the virtual touchscreen
						FirstLoop = false;
					}
					else {
						set_cursor_pos(smoothX, smoothY, 1); // set the mouse position
					}
				}
			}
			else {
				set_cursor_pos(smoothX, smoothY, 0); // free the real mouse
			}
		}
	}
	ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd); // disconnect the virtual touchscreen
	return 0;
}
