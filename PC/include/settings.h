#pragma once

#include <stdbool.h>

struct settings {
	int port;
	double smooth;
	bool Custom_Active_Zone;
    bool debug;
    bool TapFeature;
};

struct StartCoor {
    int x;
    int y;
};

struct ScreenStartCoor {
    int x;
    int y;
};

struct EndCoor {
    int x;
    int y;
};

struct ScreenEndCoor {
    int x;
    int y;
};


struct touch {
	unsigned short x;
	unsigned short y;
};

extern struct touch currentTouch;

extern struct settings settings;

extern struct StartCoor StartCoor;

extern struct EndCoor EndCoor;

extern struct ScreenStartCoor ScreenStartCoor;

extern struct ScreenEndCoor ScreenEndCoor;

void load_settings(const char *filename);