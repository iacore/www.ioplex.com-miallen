#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include "domc.h"

static struct codepair {
	unsigned short keysym;
	unsigned int domvk;
} keysymtab[] = {
	{ XK_Linefeed, DOM_VK_ENTER },
	{ XK_Pause, DOM_VK_PAUSE },
	{ XK_Scroll_Lock, DOM_VK_SCROLL_LOCK },
	{ XK_Escape, DOM_VK_ESCAPE },
	{ XK_Home, DOM_VK_HOME },
	{ XK_Left, DOM_VK_LEFT },
	{ XK_Up, DOM_VK_UP },
	{ XK_Right, DOM_VK_RIGHT },
	{ XK_Down, DOM_VK_DOWN },
	{ XK_Page_Up, DOM_VK_PAGE_UP },
	{ XK_Page_Down, DOM_VK_PAGE_DOWN },
	{ XK_End, DOM_VK_END },
	{ XK_Print, DOM_VK_PRINTSCREEN },
	{ XK_Insert, DOM_VK_INSERT },
	{ XK_Num_Lock, DOM_VK_NUM_LOCK },
	{ XK_KP_Enter, DOM_VK_ENTER },
	{ XK_KP_F1, DOM_VK_F1 },
	{ XK_KP_F2, DOM_VK_F2 },
	{ XK_KP_F3, DOM_VK_F3 },
	{ XK_KP_F4, DOM_VK_F4 },
	{ XK_KP_Home, DOM_VK_HOME },
	{ XK_KP_Left, DOM_VK_LEFT },
	{ XK_KP_Up, DOM_VK_UP },
	{ XK_KP_Right, DOM_VK_RIGHT },
	{ XK_KP_Down, DOM_VK_DOWN },
	{ XK_KP_Page_Up, DOM_VK_PAGE_UP },
	{ XK_KP_Page_Down, DOM_VK_PAGE_DOWN },
	{ XK_KP_End, DOM_VK_END },
	{ XK_KP_Insert, DOM_VK_INSERT },
	{ XK_KP_Delete, DOM_VK_DELETE },
	{ XK_KP_0, 0x0030 },
	{ XK_KP_1, 0x0031 },
	{ XK_KP_2, 0x0032 },
	{ XK_KP_3, 0x0033 },
	{ XK_KP_4, 0x0034 },
	{ XK_KP_5, 0x0035 },
	{ XK_KP_6, 0x0036 },
	{ XK_KP_7, 0x0037 },
	{ XK_KP_8, 0x0038 },
	{ XK_KP_9, 0x0039 },
	{ XK_F1, DOM_VK_F1 },
	{ XK_F2, DOM_VK_F2 },
	{ XK_F3, DOM_VK_F3 },
	{ XK_F4, DOM_VK_F4 },
	{ XK_F5, DOM_VK_F5 },
	{ XK_F6, DOM_VK_F6 },
	{ XK_F7, DOM_VK_F7 },
	{ XK_F8, DOM_VK_F8 },
	{ XK_F9, DOM_VK_F9 },
	{ XK_F10, DOM_VK_F10 },
	{ XK_F11, DOM_VK_F11 },
	{ XK_F12, DOM_VK_F12 },
	{ XK_F13, DOM_VK_F13 },
	{ XK_F14, DOM_VK_F14 },
	{ XK_F15, DOM_VK_F15 },
	{ XK_F16, DOM_VK_F16 },
	{ XK_F17, DOM_VK_F17 },
	{ XK_F18, DOM_VK_F18 },
	{ XK_F19, DOM_VK_F19 },
	{ XK_F20, DOM_VK_F20 },
	{ XK_F21, DOM_VK_F21 },
	{ XK_F22, DOM_VK_F22 },
	{ XK_F23, DOM_VK_F23 },
	{ XK_F24, DOM_VK_F24 },
	{ XK_Shift_L, DOM_VK_LEFT_SHIFT },
	{ XK_Shift_R, DOM_VK_RIGHT_SHIFT },
	{ XK_Control_L, DOM_VK_LEFT_CONTROL },
	{ XK_Control_R, DOM_VK_RIGHT_CONTROL },
	{ XK_Caps_Lock, DOM_VK_CAPS_LOCK },
	{ XK_Meta_L, DOM_VK_LEFT_META },
	{ XK_Meta_R, DOM_VK_RIGHT_META },
	{ XK_Alt_L, DOM_VK_LEFT_ALT },
	{ XK_Alt_R, DOM_VK_RIGHT_ALT },
	{ XK_Delete, DOM_VK_DELETE }
};

unsigned int
keysym2domvk(unsigned short keysym)
{
	int min = 0;
	int max = sizeof(keysymtab) / sizeof(struct codepair) - 1;
	int mid;

	/* binary search in table */
	while (max >= min) {
		mid = (min + max) / 2;
		if (keysymtab[mid].keysym < keysym)
			min = mid + 1;
		else if (keysymtab[mid].keysym > keysym)
			max = mid - 1;
		else {
			/* found it */
			return keysymtab[mid].domvk;
		}
	}
	/* no matching virtKeyVal found */
	return -1;
}

