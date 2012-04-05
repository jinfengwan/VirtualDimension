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

#ifndef __SHELLHOOK_H__
#define __SHELLHOOK_H__

class ShellHook
{
public:
   ShellHook(HWND hWnd);
   ~ShellHook(void);

   static UINT WM_SHELLHOOK;

   enum ShellNotifications {
      WINDOWCREATED = 1,
      WINDOWDESTROYED = 2,
      ACTIVATESHELLWINDOW = 3,
      WINDOWACTIVATED = 4,
      GETMINRECT = 5,
      REDRAW = 6,
      TASKMAN = 7,
      LANGUAGE = 8,
      SYSMENU = 9,
      ENDTASK = 10,
      ACCESSIBILITYSTATE = 11,
      APPCOMMAND = 12,
      WINDOWREPLACED = 13,
      WINDOWREPLACING = 14,
      FLASH = (0x8000 | REDRAW),
      RUDEAPPACTIVATEED = (0x8000 | WINDOWACTIVATED)
   };

protected:
   typedef BOOL WINAPI RegisterShellHookProc(HWND,DWORD);

   static HINSTANCE hinstDLL;
   static int nbInstance;
   static RegisterShellHookProc *RegisterShellHook;

   HWND m_hWnd;
};

#endif /*__SHELLHOOK_H__*/
