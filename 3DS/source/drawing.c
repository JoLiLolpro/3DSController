#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <3ds.h>

#include "drawing.h"

void drawPixelRGBFramebuffer(u8 *fb, int x, int y, u8 r, u8 g, u8 b) {
	y = 240 - y;
	x = x;
	u32 v = (y + x * 240) * 3;
	u8 *topLeftFB = fb ? fb : gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	topLeftFB[v] = (b >> 3) + ((g & 0x1C) << 3);
	topLeftFB[v+1] = ((g & 0xE0) >> 5) + (r & 0xF8);
}

inline void drawBoxFramebuffer(u8 *fb, int x, int y, int width, int height, u8 r, u8 g, u8 b) {
	int lx, ly;
	for(lx = x; lx < x + width; lx++) {
		for(ly = y; ly < y + height; ly++) {
			drawPixelRGBFramebuffer(fb, lx, ly, r, g, b);
		}
	}
}

static u32 brightnessMain;
static u32 brightnessSub;

void disableBacklight() {
	u32 off = 0;
	
	GSPGPU_ReadHWRegs(REG_LCDBACKLIGHTMAIN, &brightnessMain, 4);
	GSPGPU_ReadHWRegs(REG_LCDBACKLIGHTSUB, &brightnessSub, 4);
	
	GSPGPU_WriteHWRegs(REG_LCDBACKLIGHTMAIN, &off, 4);
	GSPGPU_WriteHWRegs(REG_LCDBACKLIGHTSUB, &off, 4);
}

void enableBacklight() {
	GSPGPU_WriteHWRegs(REG_LCDBACKLIGHTMAIN, &brightnessMain, 4);
	GSPGPU_WriteHWRegs(REG_LCDBACKLIGHTSUB, &brightnessSub, 4);
}
