#pragma once

#include <stdbool.h>

struct settings {
	char IPString[16];
	int port;
	int BackLight;
};

extern struct settings settings;
extern struct settings defaultSettings;

extern Handle fileHandle;

bool readSettings(void);
