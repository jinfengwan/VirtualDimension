/*
 * Virtual Dimension -  a free, fast, and feature-full virtual desktop manager
 * for the Microsoft Windows platform.
 * Copyright (C) 2003-2008 Francois Ferrand
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "stdafx.h"
#include <map>
#include "SharedMenuBuffer.h"
#include "HookDLL.h"

//First, some data shared by all instances of the DLL
#ifdef __GNUC__
HWND hVDWnd __attribute__((section (".shared"), shared)) = NULL;
ATOM g_aPropName __attribute__((section (".shared"), shared)) = 0;
int g_iProcessCount __attribute__((section (".shared"), shared)) = 0;
UINT g_uiHookMessageId __attribute__((section (".shared"), shared)) = 0;
UINT g_uiShellHookMsg __attribute__((section (".shared"), shared)) = 0;
#else
HWND hVDWnd = NULL;
ATOM g_aPropName = 0;
int g_iProcessCount = 0;
UINT g_uiHookMessageId = 0;
UINT g_uiShellHookMsg = 0;
#endif

using namespace std;

#define HOOKDLL_API __declspec(dllexport)

HOOKDLL_API DWORD WINAPI doHookWindow(HWND hWnd, int data);
HOOKDLL_API DWORD WINAPI doUnHookWindow(DWORD data, HWND hWnd);
HMENU SetupMenu(HWND hWnd);
void CleanupMenu(HWND hWnd, HMENU hMenu);
void InitPopupMenu(HWND hWnd, HMENU hMenu);
int FindMenuItem(UINT cmdid, HMENU hMenu);

struct HWNDHookData
{
   WNDPROC m_fnPrevWndProc;
   int m_iData;
   HANDLE m_hMutex;
   bool m_bHookWndProcCalled;
	HMENU m_hSubMenu;
};

map<HWND,HWNDHookData*> m_HookData;

#define VDM_SYSBASE 0xBA50

UINT VDtoSysItemID(UINT id)   { return VDM_SYSBASE + (id<<4); }
UINT SystoVDItemID(UINT msg)  { return (msg - VDM_SYSBASE) >> 4; }

LRESULT CALLBACK hookWndProcW(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   HWNDHookData * pData;
   LRESULT res = 0;
   UINT_PTR syscmd;

   //Get the hook information
   pData = (HWNDHookData*)GetPropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
   if (!pData)
      return 0;

   //Gain access to the data
   WaitForSingleObject(pData->m_hMutex, INFINITE);

   //Mark that the hook procedure has been called
   pData->m_bHookWndProcCalled = true;

   //Process the message
   switch(message)
   {
   case WM_SYSCOMMAND:
      syscmd = wParam&0xfff0;
      switch(syscmd)
      {
		case SC_MINIMIZE:
         //Minimize using VD
         if (SendMessageW(hVDWnd, WM_VD_CHECK_MIN_TO_TRAY, 0, (WPARAM)pData->m_iData))
            res = PostMessageW(hVDWnd, WM_VD_HOOK_MENU_COMMAND, VDM_MINIMIZE, (WPARAM)pData->m_iData);
         break;

      case SC_MAXIMIZE:
         {
            short shift = GetKeyState(VK_SHIFT) & 0x8000;
            short ctrl = GetKeyState(VK_CONTROL) & 0x8000;

            if (shift && !ctrl)
               //Maximize width using VD
               res = PostMessageW(hVDWnd, WM_VD_HOOK_MENU_COMMAND, VDM_MAXIMIZEWIDTH, (WPARAM)pData->m_iData);
            else if (ctrl && !shift)
               //Maximize height using VD
               res = PostMessageW(hVDWnd, WM_VD_HOOK_MENU_COMMAND, VDM_MAXIMIZEHEIGHT, (WPARAM)pData->m_iData);
         }
         break;

		case SC_CLOSE:
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				SetForegroundWindow(hVDWnd);
				res = PostMessageW(hVDWnd, WM_VD_HOOK_MENU_COMMAND, VDM_KILL, (WPARAM)pData->m_iData);
			}
			break;

      default:
         if (pData->m_hSubMenu && FindMenuItem(syscmd, pData->m_hSubMenu) != -1)
			{
		 		SetForegroundWindow(hVDWnd);
            res = PostMessageW(hVDWnd, WM_VD_HOOK_MENU_COMMAND, SystoVDItemID(syscmd), (WPARAM)pData->m_iData);
			}
         break;
      }
      break;

	case WM_INITMENUPOPUP:
		if (pData->m_hSubMenu && (HMENU)wParam == pData->m_hSubMenu)
			InitPopupMenu(hWnd, (HMENU)wParam);
		break;

   case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_INACTIVE)
         PostMessageW(hVDWnd, g_uiShellHookMsg, HSHELL_WINDOWACTIVATED, (LPARAM)hWnd);
      break;

	case WM_NCMBUTTONDOWN:
		SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
		break;

	case WM_ENTERSIZEMOVE:
		PostMessageW(hVDWnd, WM_VD_WNDSIZEMOVE, (WPARAM)hWnd, TRUE);
		break;

	case WM_EXITSIZEMOVE:
		PostMessageW(hVDWnd, WM_VD_WNDSIZEMOVE, (WPARAM)hWnd, FALSE);
		break;

   case WM_DESTROY:
	   {
			//Restore window procedure
			SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)pData->m_fnPrevWndProc);

			//Alert VD window
			PostMessageW(hVDWnd, g_uiShellHookMsg, HSHELL_WINDOWDESTROYED, (LPARAM)hWnd);
		}
		break;
   }

   if (res == 0)
      res = CallWindowProcW(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);

   //Release the mutex to give back access to the data
   ReleaseMutex(pData->m_hMutex);

   return res;
}

LRESULT CALLBACK hookWndProcA(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   HWNDHookData * pData;
   LRESULT res = 0;
   UINT_PTR syscmd;

   //Get the hook information
   pData = (HWNDHookData*)GetPropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
   if (!pData)
      return 0;

   //Gain access to the data
   WaitForSingleObject(pData->m_hMutex, INFINITE);

   //Mark that the hook procedure has been called
   pData->m_bHookWndProcCalled = true;

   //Process the message
   switch(message)
   {
   case WM_SYSCOMMAND:
      syscmd = wParam&0xfff0;
      switch(syscmd)
      {
      case SC_MINIMIZE:
         //Minimize using VD
         if (SendMessageA(hVDWnd, WM_VD_CHECK_MIN_TO_TRAY, 0, (WPARAM)pData->m_iData))
            res = PostMessageA(hVDWnd, WM_VD_HOOK_MENU_COMMAND, VDM_MINIMIZE, (WPARAM)pData->m_iData);
         break;

      case SC_MAXIMIZE:
		   {
				short shift = GetKeyState(VK_SHIFT) & 0x8000;
				short ctrl = GetKeyState(VK_CONTROL) & 0x8000;

				if (shift && !ctrl)
					//Maximize width using VD
					res = PostMessageA(hVDWnd, WM_VD_HOOK_MENU_COMMAND, VDM_MAXIMIZEWIDTH, (WPARAM)pData->m_iData);
				else if (ctrl && !shift)
					//Maximize height using VD
					res = PostMessageA(hVDWnd, WM_VD_HOOK_MENU_COMMAND, VDM_MAXIMIZEHEIGHT, (WPARAM)pData->m_iData);
				break;
			}

		case SC_CLOSE:
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				SetForegroundWindow(hVDWnd);
				res = PostMessageA(hVDWnd, WM_VD_HOOK_MENU_COMMAND, VDM_KILL, (WPARAM)pData->m_iData);
			}
			break;

      default:
         if (pData->m_hSubMenu && FindMenuItem(syscmd, pData->m_hSubMenu) != -1)
         {
            SetForegroundWindow(hVDWnd);
            res = PostMessageA(hVDWnd, WM_VD_HOOK_MENU_COMMAND, SystoVDItemID(syscmd), (WPARAM)pData->m_iData);
         }
         break;
      }
      break;

	case WM_INITMENUPOPUP:
		if (pData->m_hSubMenu && (HMENU)wParam == pData->m_hSubMenu)
			InitPopupMenu(hWnd, (HMENU)wParam);
		break;

   case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_INACTIVE)
         PostMessageA(hVDWnd, g_uiShellHookMsg, HSHELL_WINDOWACTIVATED, (LPARAM)hWnd);
      break;

	case WM_NCMBUTTONDOWN:
		SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
		break;

	case WM_ENTERSIZEMOVE:
		PostMessageA(hVDWnd, WM_VD_WNDSIZEMOVE, (WPARAM)hWnd, TRUE);
		break;

	case WM_EXITSIZEMOVE:
		PostMessageA(hVDWnd, WM_VD_WNDSIZEMOVE, (WPARAM)hWnd, FALSE);
		break;

   case WM_DESTROY:
	   {
			//Restore window procedure
			SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)pData->m_fnPrevWndProc);

			//Alert VD window
			PostMessageA(hVDWnd, g_uiShellHookMsg, HSHELL_WINDOWDESTROYED, (LPARAM)hWnd);
		}
		break;
	}

   if (res == 0)
      res = CallWindowProcA(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);

   //Release the mutex to give back access to the data
   ReleaseMutex(pData->m_hMutex);

   return res;
}

HOOKDLL_API DWORD WINAPI doHookWindow(HWND hWnd, int data)
{
   HWNDHookData * pHookData;
   bool unicode = IsWindowUnicode(hWnd) ? true : false;

   if (unicode)
      pHookData = (HWNDHookData*)GetPropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
   else
      pHookData = (HWNDHookData*)GetPropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
   if (pHookData)
   {
      // The window was already hooked... This is likely due to a previous VD crash
      // -> free old, unneeded resources, and re-setup various data members properly.
      WaitForSingleObject(pHookData->m_hMutex, INFINITE);
      pHookData->m_iData = data;
      ReleaseMutex(pHookData->m_hMutex);

      // Also, get the VD window again, just to be sure...
      hVDWnd = FindWindow("VIRTUALDIMENSION", NULL);

      // Special return value to tell that things went OK but we were already hooked
      return HOOK_OK_REHOOK;
   }

   //Allocate data for storing hook information
   pHookData = new HWNDHookData;
   if (!pHookData)
      return HOOK_ERROR;

   //Create mutex to sync access to hook information
   pHookData->m_hMutex = CreateMutex(NULL, TRUE, NULL);
   if (!pHookData->m_hMutex)
   {
      delete pHookData;
      return HOOK_ERROR;
   }

   pHookData->m_iData = data;
   pHookData->m_bHookWndProcCalled = false;
   pHookData->m_hSubMenu = NULL;

   //Replace the window procedure
   if (unicode)
   {
      SetPropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName), (HANDLE)pHookData);
      pHookData->m_fnPrevWndProc = (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)hookWndProcW);
   }
   else
   {
      SetPropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName), (HANDLE)pHookData);
      pHookData->m_fnPrevWndProc = (WNDPROC)SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)hookWndProcA);
   }
   if (!pHookData->m_fnPrevWndProc)
   {
      if (unicode)
			RemovePropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
		else
			RemovePropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
      CloseHandle(pHookData->m_hMutex);
      delete pHookData;
      return HOOK_ERROR;
   }

   //Store the hook
   pHookData->m_hSubMenu = SetupMenu(hWnd);
   m_HookData[hWnd] = pHookData;
   ReleaseMutex(pHookData->m_hMutex);

   return HOOK_OK;
}

HOOKDLL_API DWORD WINAPI doUnHookWindow(HINSTANCE hInstance, HWND hWnd)
{
   HWNDHookData* pData;
   BOOL unicode = IsWindowUnicode(hWnd);
   map<HWND,HWNDHookData*>::iterator it;

   it = m_HookData.find(hWnd);
   pData = (it == m_HookData.end()) ? NULL : it->second;
   if (pData &&
       (WaitForSingleObject(pData->m_hMutex, INFINITE) != WAIT_FAILED))
	{
		//Unsubclass the window
		if (unicode)
			SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)pData->m_fnPrevWndProc);
		else
			SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)pData->m_fnPrevWndProc);

		do
		{
			pData->m_bHookWndProcCalled = false;

			//Release the semaphore
			ReleaseMutex(pData->m_hMutex);

			//Let other thread run
			Sleep(1);

			//Wait till all calls to the "subclassed" window proc are finished
			WaitForSingleObject(pData->m_hMutex, INFINITE);
		}
		while(pData->m_bHookWndProcCalled);

		//Cleanup the hook information related to this window
		CloseHandle(pData->m_hMutex);
		if (pData->m_hSubMenu)
			CleanupMenu(hWnd, pData->m_hSubMenu);

      //Remove the hook data from all places where it is referenced
		if (unicode)
			RemovePropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
		else
			RemovePropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
      m_HookData.erase(it);

		delete pData;
	}

   //Free the library
   FreeLibraryAndExitThread(hInstance, TRUE);
   return FALSE; //if the previous call succeeded, it will not return
}

HMENU SetupMenu(HWND hWnd)
{
	HMENU hSubMenu = NULL;
	HMENU hMenu;

	hMenu = GetSystemMenu(hWnd, FALSE);
	if (hMenu)
	{
		hSubMenu = CreatePopupMenu();
		InsertMenu(hMenu, 0, MF_BYPOSITION | MF_POPUP | MF_STRING, (unsigned int)hSubMenu, "Virtual Dimension");
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	}

	return hSubMenu;
}

void CleanupMenu(HWND hWnd, HMENU hSubMenu)
{
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);

	if (hMenu)
	{
		//Remove the menu from the system menu of the window
		for(int i = 0; i<GetMenuItemCount(hMenu); i++)
			if (GetSubMenu(hMenu, i) == hSubMenu)
			{
				RemoveMenu(hMenu, i, MF_BYPOSITION);   //delete the menu
				RemoveMenu(hMenu, i, MF_BYPOSITION);   //delete the following separator
				break;
			}
	}

   //Release resources associated with the menu
	DestroyMenu(hSubMenu);
}

void InitPopupMenu(HWND hWnd, HMENU hMenu)
{
   HWNDHookData * pHookData;
   bool unicode = IsWindowUnicode(hWnd) ? true : false;

   if (unicode)
      pHookData = (HWNDHookData*)GetPropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
   else
      pHookData = (HWNDHookData*)GetPropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
   if (!pHookData)
      return;

   //Retrieve the menu description
   SharedMenuBuffer menuinfo;
   DWORD res;
   if (SendMessageTimeout(hVDWnd, WM_VD_PREPARE_HOOK_MENU, (WPARAM)menuinfo.GetFileMapping(), (LPARAM)pHookData->m_iData,
                          SMTO_ABORTIFHUNG|SMTO_NORMAL, 20000 /*20s*/, &res) &&
       res)
   {
      //Clear the menu
      while(GetMenuItemCount(hMenu) > 0)
         RemoveMenu(hMenu, 0, MF_BYPOSITION);

      //Build the menu as in the retrieved description
      menuinfo.ReadMenu(hMenu, VDtoSysItemID);
   }
}

int FindMenuItem(UINT cmdid, HMENU hMenu)
{
   for(int i = 0; i<GetMenuItemCount(hMenu); i++)
      if (GetMenuItemID(hMenu, i) == cmdid)
         return i;

   return -1;
}

extern "C"
BOOL APIENTRY DllMain( HANDLE /*hModule*/,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
      if (g_iProcessCount == 0)
		{
         g_aPropName = GlobalAddAtom("Virtual Dimension hook data property");
			g_uiHookMessageId = RegisterWindowMessage("Virtual Dimension Message");
			g_uiShellHookMsg = RegisterWindowMessage(TEXT("SHELLHOOK"));
		}
      //update hVDWnd each time, so that it may work again after a VD crash
		hVDWnd = FindWindow("VIRTUALDIMENSION", NULL);
      g_iProcessCount++;
      break;

   case DLL_PROCESS_DETACH:
      g_iProcessCount--;
      if (g_iProcessCount == 0)
         GlobalDeleteAtom(g_aPropName);
      break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
      break;
	}

   return TRUE;
}
