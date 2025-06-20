#include "wireless.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

void error(const char *functionName) {
    char errorMsg[128];
    memset(errorMsg, 0, sizeof(errorMsg));

    snprintf(errorMsg, sizeof(errorMsg), "Call to %s returned error %d (%s)", functionName, errno, strerror(errno));

    perror(errorMsg);

    if (client >= 0) {
        close(client);
    }
    if (listener >= 0) {
        close(listener);
    }

    exit(EXIT_FAILURE);
}
