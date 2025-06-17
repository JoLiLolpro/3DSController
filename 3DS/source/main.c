#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <setjmp.h>

#include <3ds.h>

#include "wireless.h"
#include "settings.h"
#include "drawing.h"


static jmp_buf exitJmp;

void hang(char *message) {
	while(aptMainLoop()) {
		hidScanInput();
		
		clearScreen();
		drawString(10, 10, "%s", message);
		drawString(10, 20, "Press Start and Select to exit.");
		
		u32 kHeld = hidKeysHeld();
		if((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
		
		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}
}

void drawRect(u16* fb, int fbWidth, int x, int y, int w, int h, u16 color) {
    for (int j = 0; j < h; j++) {
        u16* row = fb + (y + j) * fbWidth + x;
        for (int i = 0; i < w; i++) {
            row[i] = color;
        }
    }
}

int main(void) {
	acInit();
	gfxInitDefault();
	
	gfxSetDoubleBuffering(GFX_TOP, false);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);
	
	if(setjmp(exitJmp)) goto exit;
	static touchPosition lastTouch = {0xFFFF, 0xFFFF};
	
	clearScreen();
	drawString(10, 10, "Initialising FS...");
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	fsInit();
	
	clearScreen();
	drawString(10, 10, "Initialising SOC...");
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	socInit((u32 *)memalign(0x1000, 0x100000), 0x100000);
	
	while(aptMainLoop()) { /* Wait for WiFi; break when WiFiStatus is truthy */
		u32 wifiStatus = 0;
		ACU_GetWifiStatus(&wifiStatus);
		if(wifiStatus) break;
		
		hidScanInput();
		clearScreen();
		drawString(10, 10, "Waiting for WiFi connection...");
		drawString(10, 20, "Ensure you are in range of an access point,");
		drawString(10, 30, "and that wireless communications are enabled.");
		drawString(10, 50, "You can alternatively press Start and Select to exit.");
		
		u32 kHeld = hidKeysHeld();
		if((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
		
		gfxFlushBuffers();
		gfxSwapBuffers();
	}
	
	clearScreen();
	drawString(10, 10, "Reading settings...");
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	if(!readSettings()) {
		hang("Could not read 3DSController.ini!");
	}
	
	clearScreen();
	drawString(10, 10, "Connecting to %s on port %d...", settings.IPString, settings.port);
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	openSocket(settings.port);
	sendConnectionRequest();

	int XS = 0;
	int YS;
	int XE;
	int YE;

	while (receiveBuffer(sizeof(struct packet)) < 0) {

	}
	if (rcvBuf.header.command == CONNECT) {
		XS = rcvBuf.Connect.activeZoneStart.x;
		YS = rcvBuf.Connect.activeZoneStart.y;
		XE = rcvBuf.Connect.activeZoneEnd.x;
		YE = rcvBuf.Connect.activeZoneEnd.y;
	}
	int CalculV = XE-XS;
	int CalculH = YE-YS;

	clearScreen();
	gfxFlushBuffers();
	gfxSwapBuffers();

	if(!settings.BackLight) disableBacklight();

	while(aptMainLoop()) {

		hidScanInput();
		
		u32 kHeld = hidKeysHeld();
		touchPosition touch;
		touchRead(&touch);

		// draw the lines on the bottom screen
		
		if(settings.BackLight) {
			drawBox(XS, YS, CalculV, 5, 0, 255, 0); // Top vertinal
			drawBox(XS, YS, 5, CalculH, 0, 255, 0); // Left horizontal
			drawBox(XS, YE, CalculV+5, 5, 0, 255, 0); // Bottom vertical
			drawBox(XE, YS, 5, CalculH, 0, 255, 0); // Right horizontal
		}

		if (touch.px != lastTouch.px || touch.py != lastTouch.py) {
			sendKeys(kHeld, touch);
			lastTouch = touch;
		}
		
		if((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
		
		gfxFlushBuffers();
		gfxSwapBuffers();
	}
	
	exit:

	if(!settings.BackLight) enableBacklight();
	
	SOCU_ShutdownSockets();
	
	svcCloseHandle(fileHandle);
	fsExit();
	gfxExit();
	acExit();
	
	return 0;
}