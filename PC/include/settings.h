#pragma once

#include <stdbool.h>

struct settings {
	int port;
	double smooth;
};

extern struct settings settings;
extern struct settings defaultSettings;

bool readSettings(void);
