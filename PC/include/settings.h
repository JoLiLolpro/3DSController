#pragma once

#include <stdbool.h>

struct settings {
	int port;
	double smooth;
	bool Custom_Active_Zone;
    bool debug;
};

struct StartCoor {
    int x;
    int y;
};

struct EndCoor {
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

void load_settings(const char *filename);