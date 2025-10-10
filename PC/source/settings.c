#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "wireless.h"
#include "cJSON.h"
#include "settings.h"
#include "error.h"

struct settings settings;
struct StartCoor StartCoor;
struct EndCoor EndCoor;
struct ScreenStartCoor ScreenStartCoor;
struct ScreenEndCoor ScreenEndCoor;
struct touch currentTouch;

void load_settings(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        error("Failed to open the settings file\n");
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
        error("Error parsing JSON\n");
        free(json_data);
        return;
    }

    // Update settings struct with JSON values
    cJSON *port = cJSON_GetObjectItem(json, "port");
    cJSON *smooth = cJSON_GetObjectItem(json, "smooth");
    cJSON *customZone = cJSON_GetObjectItem(json, "Custom_Active_Zone");
	cJSON *sc = cJSON_GetObjectItem(json, "Start_Coordinate");
	cJSON *ec = cJSON_GetObjectItem(json, "End_Coordinate");
    cJSON *esc = cJSON_GetObjectItem(json, "ScreenStartCoor");
    cJSON *eec = cJSON_GetObjectItem(json, "ScreenEndCoor");
    cJSON *debug = cJSON_GetObjectItem(json, "Debug");
    cJSON *TapFeature = cJSON_GetObjectItem(json, "Drawing");

    if (cJSON_IsNumber(port)) settings.port = port->valueint;
    if (cJSON_IsNumber(smooth)) settings.smooth = smooth->valuedouble;
    if (cJSON_IsBool(customZone)) settings.Custom_Active_Zone = cJSON_IsTrue(customZone);
    if (cJSON_IsBool(debug)) settings.debug = cJSON_IsTrue(debug);
    if (cJSON_IsBool(TapFeature)) settings.TapFeature = cJSON_IsTrue(TapFeature);

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
    if (cJSON_IsObject(esc)) {
    	cJSON *EXs = cJSON_GetObjectItem(esc, "x");
    	cJSON *EYs = cJSON_GetObjectItem(esc, "y");
    	if (cJSON_IsNumber(EXs) && cJSON_IsNumber(EYs)) {
    	    ScreenStartCoor.x = EXs->valuedouble;
    	    ScreenStartCoor.y = EYs->valuedouble;
    	}
	}

    if (cJSON_IsObject(eec)) {
    	cJSON *EXe = cJSON_GetObjectItem(eec, "x");
    	cJSON *EYe = cJSON_GetObjectItem(eec, "y");
    	if (cJSON_IsNumber(EXe) && cJSON_IsNumber(EYe)) {
    	    ScreenEndCoor.x = EXe->valuedouble;
    	    ScreenEndCoor.y = EYe->valuedouble;
    	}
	}

    cJSON_Delete(json);
    free(json_data);
}