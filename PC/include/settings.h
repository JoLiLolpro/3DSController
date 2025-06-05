#pragma once

#include <stdbool.h>

struct settings {
	int port;
	int throttle;
};

extern struct settings settings;
extern struct settings defaultSettings;

bool readSettings(void);
