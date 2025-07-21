#pragma once

#include <stdbool.h>

struct settings {
	int port;
	double smooth;
	bool Custom_Active_Zone;
    bool debug;
};

struct StartCoor {
    double x;
    double y;
};

struct EndCoor {
    double x;
    double y;
};

struct touch {
	short x;
	short y;
};

extern struct touch currentTouch;

extern struct settings settings;

extern struct StartCoor StartCoor;

extern struct EndCoor EndCoor;

void load_settings(const char *filename);