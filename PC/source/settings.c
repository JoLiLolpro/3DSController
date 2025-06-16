#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "keys.h"
#include "wireless.h"
#include "cJSON.h"
#include "settings.h"

struct settings settings;
struct StartCoor StartCoor;
struct EndCoor EndCoor;

void load_settings(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open the settings file\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *json_data = (char *)malloc(file_size + 1);
    fread(json_data, 1, file_size, file);
    json_data[file_size] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(json_data);
    if (!json) {
        printf("Error parsing JSON\n");
        free(json_data);
        return;
    }

    // Update settings struct with JSON values
    cJSON *port = cJSON_GetObjectItem(json, "port");
    cJSON *smooth = cJSON_GetObjectItem(json, "smooth");
    cJSON *customZone = cJSON_GetObjectItem(json, "Custom_Active_Zone");
	cJSON *sc = cJSON_GetObjectItem(json, "Start_Coordinate");
	cJSON *ec = cJSON_GetObjectItem(json, "End_Coordinate");

    if (cJSON_IsNumber(port)) settings.port = port->valueint;
    if (cJSON_IsNumber(smooth)) settings.smooth = smooth->valuedouble;
    if (cJSON_IsBool(customZone)) settings.Custom_Active_Zone = cJSON_IsTrue(customZone);

	if (cJSON_IsObject(sc)) {
    	cJSON *Xs = cJSON_GetObjectItem(sc, "x");
    	cJSON *Ys = cJSON_GetObjectItem(sc, "y");
    	if (cJSON_IsNumber(Xs) && cJSON_IsNumber(Ys)) {
    	    StartCoor.x = Xs->valuedouble;
    	    StartCoor.y = Ys->valuedouble;
    	}
	}

	if (cJSON_IsObject(ec)) {
    	cJSON *Xe = cJSON_GetObjectItem(ec, "x");
    	cJSON *Ye = cJSON_GetObjectItem(ec, "y");
    	if (cJSON_IsNumber(Xe) && cJSON_IsNumber(Ye)) {
    	    EndCoor.x = Xe->valuedouble;
    	    EndCoor.y = Ye->valuedouble;
    	}
	}	

    cJSON_Delete(json);
    free(json_data);
}