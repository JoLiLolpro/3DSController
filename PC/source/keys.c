#include "keys.h"

// Sideband comunication with vJoy Device
//{781EF630-72B2-11d2-B852-00C04FAD5101}
DEFINE_GUID(GUID_DEVINTERFACE_VJOY, 0x781EF630, 0x72B2, 0x11d2, 0xB8, 0x52, 0x00, 0xC0, 0x4F, 0xAD, 0x51, 0x01);

unsigned int lastKeys;
unsigned int currentKeys;

struct touch lastTouch;
struct touch currentTouch;