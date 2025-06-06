#include <stdio.h>
#include <malloc.h>

#include <3ds.h>

#include "wireless.h"

#include "settings.h"

#include "inet_pton.h"

struct settings settings;

struct settings defaultSettings = {
	IPString: "",
	port: DEFAULT_PORT,
	BackLight: 1,
};

Handle fileHandle;

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
	char *buffer = NULL;
	u64 size;
	u32 bytesRead;
	
	FS_Path filePath = fsMakePath(PATH_ASCII, "/3DSController.ini");
	
	Result ret = FSUSER_OpenFileDirectly(&fileHandle, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""), filePath, FS_OPEN_READ, 0x00000000);
	if(ret) return false;
	
	ret = FSFILE_GetSize(fileHandle, &size);
	if(ret) return false;
	
	buffer = malloc(size);
	if(!buffer) return false;
	
	ret = FSFILE_Read(fileHandle, &bytesRead, 0x0, buffer, size);
	if(ret || size != bytesRead) return false;
	
	ret = FSFILE_Close(fileHandle);
	if(ret) return false;
	
	memcpy(&settings, &defaultSettings, sizeof(struct settings));
	
	char setting[64] = { '\0' };
	
	if(getSetting("IP: ", buffer, settings.IPString)) {
		//inet_pton(AF_INET, settings.IPString, &(saout.sin_addr));
		inet_pton4(settings.IPString, (unsigned char *)&(saout.sin_addr));
	}
	else {
		free(buffer);
		return false;
	}
	
	if(getSetting("Port: ", buffer, setting)) {
		sscanf(setting, "%d", &settings.port);
	}

	if(getSetting("BackLight: ", buffer, setting)) {
		sscanf(setting, "%1d", &settings.BackLight);
	}
	
	free(buffer);
	
	return true;
}