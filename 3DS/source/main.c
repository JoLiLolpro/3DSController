#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <setjmp.h>
#include <stdarg.h>
#include <unistd.h>

#include <3ds.h>

#include "wireless.h"
#include "settings.h"
#include "drawing.h"
#include "inet_pton.h"


static jmp_buf exitJmp;
int LinePosition = 2;
bool current_backlight = true;

static double VERSION = 1.7;

void printText(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    printf("\x1b[%d;2H%s", LinePosition, buffer);
    LinePosition += 2;
	gfxFlushBuffers();
	gfxSwapBuffers();
}


void hang(char *message) {

	printText("%s", message);
	printText("Press Start and Select to exit.");

	while(aptMainLoop()) {
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) longjmp(exitJmp, 1);
	}
}

int main(void) {
	acInit();
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	
	gfxSetDoubleBuffering(GFX_TOP, false);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);

	int XS;
	int YS;
	int XE;
	int YE;

	static SwkbdState swkbd;
	static char ipInput[16];
	static SwkbdStatusData swkbdStatus;
	static SwkbdLearningData swkbdLearning;
	SwkbdButton button = SWKBD_BUTTON_NONE;

	if(setjmp(exitJmp)) goto exit;

	printText("Welcome to 3DS Mouse, version %.1f", VERSION);

	printText("Reading settings...");

	if(!readSettings()) {
		hang("Could not read 3DSMouse.ini!");
	}
	
	printText("Initialising FS...");
	
	fsInit();
	
	printText("Initialising SOC...");
	
	socInit((u32 *)memalign(0x1000, 0x100000), 0x100000);

	u32 wifiStatus = 0;
	ACU_GetWifiStatus(&wifiStatus);
	if (!wifiStatus) {
		printText("Waiting for WiFi connection...");
		printText("Ensure you are in range of an access point,");
		printText("and that wireless communications are enabled.");
		printText("press Start to exit.");
		while(aptMainLoop()) { /* Wait for WiFi; break when WiFiStatus is truthy */
			u32 wifiStatus = 0;
			ACU_GetWifiStatus(&wifiStatus);
			if(wifiStatus) break;
	
			hidScanInput();
			u32 kDown = hidKeysDown();

			if (kDown & KEY_START) longjmp(exitJmp, 1);
		
		}
	}

	// ask for an IP

	printText("");
	printText("A: IP from the settings file (%s)", settings.IPString);
	printText("B: choose a custom IP");

	while (true) {
		hidScanInput();
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) longjmp(exitJmp, 1);

		if (kDown & KEY_A) {
			break;
		}

		if (kDown & KEY_B) {
			swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 3, -1);
			swkbdSetInitialText(&swkbd, ipInput);
			swkbdSetHintText(&swkbd, "Exemple: 192.168.1.1");
			swkbdSetFeatures(&swkbd, SWKBD_PREDICTIVE_INPUT);
			static bool reload = false;
			swkbdSetStatusData(&swkbd, &swkbdStatus, reload, true);
			swkbdSetLearningData(&swkbd, &swkbdLearning, reload, true);
			reload = true;
			button = swkbdInputText(&swkbd, ipInput, sizeof(ipInput));


			if (inet_pton4(ipInput, (unsigned char *)&(saout.sin_addr))) {
				strncpy(settings.IPString, ipInput, sizeof(settings.IPString));
				settings.IPString[sizeof(settings.IPString) - 1] = '\0';
				break;
			} else {
				printText("wrong ip format please try again by pressing B");
			}
		}
	}


	printText("Connecting to %s on port %d...", settings.IPString, settings.port);
	
	openSocket(settings.port);

	// wait for the pc response

	while (receiveBuffer(sizeof(struct packet)) <= 0) {
		hidScanInput();
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) longjmp(exitJmp, 1);
		sendConnectionRequest();
	}

	if (rcvBuf.header.command == CONNECT) {
		XS = rcvBuf.Connect.activeZoneStart.x;
		YS = rcvBuf.Connect.activeZoneStart.y;
		XE = rcvBuf.Connect.activeZoneEnd.x;
		YE = rcvBuf.Connect.activeZoneEnd.y;
	}

	int CalculV = XE-XS;
	int CalculH = YE-YS;
	YS = YS+1;

	printText("Connected!");

	if(!settings.BackLight) printText("disabling backlight in 5 seconds");

	// draw the lines on the bottom screen, DONT ASK ME HOW I FOUND THOSES NUMBERS

	drawBox(XS, YS, CalculV, 5, 0, 255, 0); // Top horizontal
	drawBox(XS, YS, 5, CalculH, 0, 255, 0); // Left vertical
	drawBox(XS, YE-4, CalculV, 5, 0, 255, 0); // Bottom horizontal
	drawBox(XE-5, YS, 5, CalculH, 0, 255, 0); // Right vertical
	gfxFlushBuffers();
	gfxSwapBuffers();

	if(!settings.BackLight) {
		sleep(5);
		disableBacklight();
		current_backlight = false;
	}

	// main loop that send the touch data

	while(aptMainLoop()) {

		hidScanInput();
		
		u32 kDown = hidKeysDown();
		touchPosition touch;
		touchRead(&touch);

		sendKeys(kDown, touch);

		if (kDown & KEY_START) longjmp(exitJmp, 1);
	}

	
	exit:
	
	if(!settings.BackLight && !current_backlight) enableBacklight();
	
	SOCU_ShutdownSockets();
	svcCloseHandle(fileHandle);
	fsExit();
	gfxExit();
	acExit();
	
	return 0;
}