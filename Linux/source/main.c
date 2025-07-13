// 3DS Controller Server

#define VERSION 0.6

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
#include "keys.h"
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
	
	printf("3DS Controller Server %.1f\n", VERSION);

	Display *display = XOpenDisplay(NULL);
	if (!display) {
    	fprintf(stderr, RED_TEXT "Error: Could not open X display.\n" NORMAL_TEXT);
    	return 1;
	}

	Screen *s = DefaultScreenOfDisplay(display);
	unsigned int screenWidth = s->width;
	unsigned int screenHeight = s->height;


	double widthMultiplier = 0.0;
	double heightMultiplier = 0.0;

	static double smoothX = -1, smoothY = -1;
	
	static bool Debug = false; // if this is true the latency will be displayed

	struct timespec receiveTime, applyTime;
	double latencyMs;
	static bool FirstLoop = true;

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
	printf("Smoothing: %f\n", settings.smooth);
	printf("Port: %d\n", settings.port);
	printf("Running on: %s\n", hostName);
	printf("Your local IP(s):\n");
	printIPs();
	printf("\n");
	
	startListening();

	printf("listening\n");

	while (1) {
		if (receiveBuffer(sizeof(struct packet)) > 0) {
			switch (buffer.header.command) {
				case CONNECT:
					currentTouch.x = 0;
					currentTouch.y = 0;

					buffer.header.command = CONNECT;
					printf("3DS Connected!\n");

					usleep(50000);
					sendBuffer(1, StartX, StartY, EndX, EndY);

					usleep(50000);
					sendBuffer(1, StartX, StartY, EndX, EndY);

					usleep(50000);
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
								clock_gettime(CLOCK_MONOTONIC, &receiveTime);
								init_uinput_device(screenWidth, screenHeight);
								FirstLoop = false;
							}
							else {
								clock_gettime(CLOCK_MONOTONIC, &applyTime);
								latencyMs = (applyTime.tv_sec - receiveTime.tv_sec) * 1000.0 + (applyTime.tv_nsec - receiveTime.tv_nsec) / 1000000.0;
								
								if (latencyMs > 30) {
									printf(RED_TEXT "Latency: %.3f ms\n" NORMAL_TEXT, latencyMs);
								}
								else {
									printf("Latency: %.3f ms\n", latencyMs);
								}

								set_cursor_pos(smoothX, smoothY, 1);

								clock_gettime(CLOCK_MONOTONIC, &receiveTime);
							}
						}
						else {
							if (FirstLoop) {
								init_uinput_device(screenWidth, screenHeight);
								FirstLoop = false;
							}
							else {
								set_cursor_pos(smoothX, smoothY, 1);
							}
						}
					}
					else {
						set_cursor_pos(smoothX, smoothY, 0); // free the real mouse
					}
					
					break;
			}
		}
	}

	ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);
	error("accept()");
	return 0;
}
