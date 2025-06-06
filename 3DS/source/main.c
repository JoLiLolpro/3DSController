#include <stdlib.h>
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
	
	clearScreen();
	gfxFlushBuffers();
	gfxSwapBuffers();

	if(!settings.BackLight) disableBacklight();

	while(aptMainLoop()) {
		hidScanInput();
		irrstScanInput();
		
		u32 kHeld = hidKeysHeld();
		touchPosition touch;
		touchRead(&touch);
		
		// draw the lines on the bottom screen
		if(settings.BackLight) {
			drawBox(155, 115, 5, 126, 0, 255, 0); //vertical
			drawBox(160, 115, 160, 5, 0, 255, 0); //horizontal
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