#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "keys.h"
#include "wireless.h"

#include "settings.h"

struct settings settings;

struct settings defaultSettings = {
	port: 8889,
	smooth: 0.3,
};

static bool getSetting(char *name, char *src, char *dest) {
	char *start = strstr(src, name);
	
	if(start) {
		start += strlen(name);
		
		char *end = start + strlen(start);
		if(strstr(start, "\n") - 1 < end) end = strstr(start, "\n") - 1;
		size_t size = (size_t)end - (size_t)start;
		
		strncpy(dest, start, size);
		dest[size] = '\0';
		
		return true;
	}
	
	return false;
}

bool readSettings(void) {
	FILE *f;
	size_t len = 0;
	char *buffer = NULL;
	
	memcpy(&settings, &defaultSettings, sizeof(struct settings));
	
	f = fopen("3DSController.ini", "rb");
	if(!f) {
		return false;
	}
	
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	rewind(f);
	
	buffer = malloc(len);
	if(!buffer) {
		fclose(f);
		return false;
	}
	
	fread(buffer, 1, len, f);
	
	char setting[64] = { '\0' };
	
	if(getSetting("Port: ", buffer, setting)) {
		sscanf(setting, "%d", &settings.port);
	}

	if(getSetting("Smooth: ", buffer, setting)) {
		sscanf(setting, "%lf", &settings.smooth);
	}
	
	fclose(f);
	
	return true;
}
