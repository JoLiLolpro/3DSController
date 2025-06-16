#pragma once

#include <stdbool.h>

struct settings {
	int port;
	double smooth;
	bool Custom_Active_Zone;
};

struct StartCoor {
    double x;
    double y;
};

struct EndCoor {
    double x;
    double y;
};

extern struct settings settings;

extern struct StartCoor StartCoor;

extern struct EndCoor EndCoor;

void load_settings(const char *filename);