#ifndef __DESKPOPUPMENU_H__
#define __DESKPOPUPMENU_H__

#include "stdafx.h"
#include "Window.h"
#include <Windowsx.h>
#include "Dwmapi.h"

extern WNDPROC WndProcOld;
extern HMENU hMenuPopup;
extern HWND hWndPopupMenu;
extern HWND hWndVD;
extern HWND hwndThumbnail;
extern HMENU hBaseMenuPopup;

LRESULT CALLBACK WndProcDeskPopupMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif