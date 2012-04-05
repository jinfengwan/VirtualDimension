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

#include "StdAfx.h"
#include "shellhook.h"
#include "Locale.h"
#include "Resource.h"

int ShellHook::nbInstance = 0;
HINSTANCE ShellHook::hinstDLL = NULL;

ShellHook::RegisterShellHookProc * ShellHook::RegisterShellHook = NULL;
/*
  // RegisterShellHook params
  RSH_DEREGISTER       = 0;
  RSH_REGISTER         = 1;
  RSH_REGISTER_PROGMAN = 2;
  RSH_REGISTER_TASKMAN = 3;
*/
ShellHook::ShellHook(HWND hWnd): m_hWnd(hWnd)
{
   if (nbInstance == 0)
   {
      hinstDLL = LoadLibrary((LPCTSTR) "shell32.dll");

      RegisterShellHook = (RegisterShellHookProc*)GetProcAddress(hinstDLL, (LPCSTR)181);
      if (RegisterShellHook == NULL)
      {
         locMessageBox(NULL, IDS_REGISTERSHELLHOOK_ERROR, IDS_ERROR, MB_OK);
         FreeLibrary(hinstDLL);
         return;
      }
   }

   nbInstance++;

   //The following is taken from indiestep/blackbox (they seem to use the exact same code :)
   OSVERSIONINFO verInfo;
   verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(&verInfo);

   //RegisterShellHook(NULL, TRUE);
   
   if (verInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
      RegisterShellHook(hWnd, 1); //Windows 95, Windows 98, or Windows Me.
   else
      RegisterShellHook(hWnd, 3); //Windows NT 3.51, Windows NT 4.0, Windows 2000, Windows XP, or Windows Server 2003 family.
}

ShellHook::~ShellHook(void)
{
   if (RegisterShellHook == NULL)
      return;

	//Don't know why, but recent versions of Mingw (>3.2.3) make VD crash when we
	//call FreeLibrary() a few lines down, if we call RegisterShellHook() now...
	//The fix is either to not un-register the hook, or not to free the library.
	//It also works OK if we do not use LoadLibrary(), but simply GetModuleHandle()
	//and thus do not free the library at the end...

	//RegisterShellHook(m_hWnd, 0);

   nbInstance--;
   
   if (nbInstance == 0)
		FreeLibrary(hinstDLL);
}
