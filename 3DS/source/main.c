#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <setjmp.h>
#include <stdarg.h>

#include <3ds.h>

#include "wireless.h"
#include "settings.h"
#include "drawing.h"


static jmp_buf exitJmp;
int LinePosition = 2;

static double VERSION = 1.6;

void printText(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    printf("\x1b[%d;2H%s", LinePosition, buffer);
    LinePosition += 2;
}


void hang(char *message) {

	printText("%s", message);
	printText("Press Start and Select to exit.");

	while(aptMainLoop()) {
		hidScanInput();

		u32 kHeld = hidKeysHeld();
		if((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
		
		gfxFlushBuffers();
		gfxSwapBuffers();
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
	
	if(setjmp(exitJmp)) goto exit;

	static touchPosition lastTouch = {0xFFFF, 0xFFFF};

	printText("Welcome to 3DS Mouse, version %.1f", VERSION);

	printText("Reading settings...");
	gfxFlushBuffers();
	gfxSwapBuffers();

	if(!readSettings()) {
		hang("Could not read 3DSController.ini!");
	}
	
	if(!settings.BackLight) disableBacklight();
	
	printText("Initialising FS...");
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	fsInit();
	
	printText("Initialising SOC...");
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	socInit((u32 *)memalign(0x1000, 0x100000), 0x100000);

	u32 wifiStatus = 0;
	ACU_GetWifiStatus(&wifiStatus);
	if (!wifiStatus) {
		printText("Waiting for WiFi connection...");
		printText("Ensure you are in range of an access point,");
		printText("and that wireless communications are enabled.");
		printText("You can alternatively press Start and Select to exit.");
		while(aptMainLoop()) { /* Wait for WiFi; break when WiFiStatus is truthy */
			u32 wifiStatus = 0;
			ACU_GetWifiStatus(&wifiStatus);
			if(wifiStatus) break;
	
			hidScanInput();
			u32 kHeld = hidKeysHeld();

			if((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
		
			gfxFlushBuffers();
			gfxSwapBuffers();
		}
	}

	printText("Connecting to %s on port %d...", settings.IPString, settings.port);
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	openSocket(settings.port);

	// wait for the pc response

	while (receiveBuffer(sizeof(struct packet)) < 0) {
		hidScanInput();
		u32 kHeld = hidKeysHeld();
		if((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
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

	// main loop that send the touch data

	while(aptMainLoop()) {

		hidScanInput();
		
		u32 kHeld = hidKeysHeld();
		touchPosition touch;
		touchRead(&touch);

		// draw the lines on the bottom screen, DONT ASK ME HOW I FOUND THOSES NUMBERS
		
		if(settings.BackLight) {
			drawBox(XS, YS, CalculV, 5, 0, 255, 0); // Top horizontal
			drawBox(XS, YS, 5, CalculH, 0, 255, 0); // Left vertical
			drawBox(XS, YE-4, CalculV, 5, 0, 255, 0); // Bottom horizontal
			drawBox(XE-5, YS, 5, CalculH, 0, 255, 0); // Right vertical
		}

		// send data only id the touch update

		if (touch.px != lastTouch.px || touch.py != lastTouch.py) {
			sendKeys(kHeld, touch);
			lastTouch = touch;
		}
		
		if((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
		
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	
	exit:
	
	if(!settings.BackLight && settings.found) enableBacklight();
	
	SOCU_ShutdownSockets();
	svcCloseHandle(fileHandle);
	fsExit();
	gfxExit();
	acExit();
	
	return 0;
}