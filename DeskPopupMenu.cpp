#include "StdAfx.h"
#include "DesktopManager.h"
#include "DeskPopupMenu.h"

WNDPROC WndProcOld;
HMENU hMenuPopup;
HWND hWndPopupMenu;
HWND hWndVD;
HWND hwndThumbnail;
HMENU hBaseMenuPopup;

LRESULT CALLBACK WndProcDeskPopupMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HCURSOR dragCursor;
	static Window* draggedWindow;
	switch(message)
	{
	case WM_NCHITTEST:
		{
			POINT pt;
			BOOL screenPos;
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			//screenPos = ClientToScreen(hWnd, &pt);
			int index;
			index = MenuItemFromPoint(NULL, hMenuPopup, pt);
			for(int i = 0; i < GetMenuItemCount(hMenuPopup); i++)
			{
				if(i == index)
				{
					HiliteMenuItem(hWnd, hMenuPopup, i, MF_BYPOSITION|MF_HILITE);

					HMODULE hMod = LoadLibrary("Dwmapi.dll");
					if(hMod == NULL)
						break;
					typedef HRESULT (WINAPI *LPDwmIsCompositionEnabled)(BOOL*);
					LPDwmIsCompositionEnabled LPDwmIsCompositionEnabled1;
					LPDwmIsCompositionEnabled1 = (LPDwmIsCompositionEnabled)GetProcAddress(hMod, "DwmIsCompositionEnabled");
					if(LPDwmIsCompositionEnabled1 == NULL)
						break;
					BOOL bAeroEnabled;
					HRESULT hr = (*LPDwmIsCompositionEnabled1)(&bAeroEnabled);
					if(SUCCEEDED(hr))
					{
						if(bAeroEnabled)
						{
							UINT MenuItemID = GetMenuItemID(hMenuPopup, i);
							HTHUMBNAIL thumbnail;
							HRESULT hr = DwmRegisterThumbnail(hwndThumbnail, ((Window*)(MenuItemID - WM_USER))->GetOwnedWindow(), &thumbnail);

							RECT dest = {0,0,300,300};
							DWM_THUMBNAIL_PROPERTIES dskThumbProps;
							dskThumbProps.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_OPACITY | DWM_TNP_VISIBLE | DWM_TNP_SOURCECLIENTAREAONLY;
							dskThumbProps.rcDestination = dest;
							dskThumbProps.opacity = /*(255 * 70)/100*/255;
							dskThumbProps.fVisible = TRUE;
							dskThumbProps.fSourceClientAreaOnly = TRUE; 

							hr = DwmUpdateThumbnailProperties(thumbnail,&dskThumbProps);
							int iii = 0;
						}
					}
				}
				else
				{
					HiliteMenuItem(hWnd, hMenuPopup, i, MF_BYPOSITION|MF_UNHILITE);
				}
			}
		}
		break;
	case WM_LBUTTONDOWN:
		{
			POINT pt;
			BOOL screenPos;
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			screenPos = ClientToScreen(hWnd, &pt);
			int index;
			index = MenuItemFromPoint(NULL, hMenuPopup, pt);

			MENUITEMINFO mii;
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_ID;
			BOOL bRet;
			bRet = GetMenuItemInfo(hMenuPopup, index, TRUE, &mii);
			draggedWindow = (Window*)(mii.wID - WM_USER);
			if(DragDetect(hWnd, pt))
			{
				SetCapture(hWnd);

				ICONINFO icon;
				GetIconInfo(draggedWindow->GetIcon(), &icon);
				icon.fIcon = FALSE;

				dragCursor = (HCURSOR)CreateIconIndirect(&icon);
				SetCursor(dragCursor);
			}
			else
			{
				Desktop* desk = draggedWindow->GetDesk();
				desk->OnMenuItemSelected(hMenuPopup, mii.wID);
				EndMenu();
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			if (draggedWindow == NULL)
				return 0;
			ReleaseCapture();
			DestroyCursor(dragCursor);

			POINT pt;
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			ClientToScreen(hWnd, &pt);
			HWND hWndCurrent = WindowFromPoint(pt);
			if(hWndCurrent == hWnd)
			{
				break;
			}
			else if(hWndCurrent == hWndVD)
			{
				ScreenToClient(hWndVD, &pt);
				Desktop* desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
				if (draggedWindow->IsOnDesk(desk))
				{
					::DestroyWindow(hwndThumbnail);
					BOOL bRet = EndMenu();
					break;
				}

				draggedWindow->MoveToDesktop(desk);

				InvalidateRect(hWndVD, NULL, FALSE);
			}
			else
			{
				Desktop* desk = deskMan->GetCurrentDesktop();
				draggedWindow->MoveToDesktop(desk);
				draggedWindow->Activate();
			}

		}
		EndMenu();

		break;
	}

	return CallWindowProcA(WndProcOld, hWnd, message, wParam, lParam);
}