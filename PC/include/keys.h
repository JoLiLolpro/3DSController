#pragma once

#include <windows.h>
#include <winable.h>
#include <winuser.h>

// For some reason, these are not defined in winuser.h like they used to be...
#ifndef VK_OEM_MINUS
#define VK_OEM_MINUS 0xBD
#endif
#ifndef VK_OEM_COMMA
#define VK_OEM_COMMA 0xBC
#endif
#ifndef KEYEVENTF_SCANCODE
#define KEYEVENTF_SCANCODE 0x08
#endif

#ifndef MAPVK_VK_TO_VSC
#define MAPVK_VK_TO_VSC 0
#endif

struct touch {
	short x;
	short y;
};

extern struct touch lastTouch;
extern struct touch currentTouch;