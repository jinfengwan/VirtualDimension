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

#ifndef __MOUSEWARP_H__
#define __MOUSEWARP_H__

#include "stdafx.h"

/** Polling interval for mouse position check.
 */
#define MOUSE_WRAP_DELAY_CHECK   50

class MouseWarp
{
public:
   MouseWarp();
   ~MouseWarp(void);

   void EnableWarp(bool enable);
   bool IsWarpEnabled() const             { return m_enableWarp; }

   void SetSensibility(LONG sensibility);
   void SetMinDuration(DWORD minDuration);
   void SetRewarpDelay(DWORD rewarpDelay);
   void InvertMousePos(bool invert);
   void SetWarpKey(int vkey);

   void RefreshDesktopSize();

   void Configure(HWND hParentWnd);

protected:
   static DWORD WINAPI MouseCheckThread(LPVOID lpParameter);
   static LRESULT WINAPI PropertiesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

   LRESULT OnTimer(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnMouseWarp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   enum WarpLocation {
      WARP_NONE,
      WARP_LEFT,
      WARP_RIGHT,
      WARP_TOP,
      WARP_BOTTOM,
   };

   WarpLocation m_warpLocation;
   UINT_PTR m_timerId;

   HANDLE m_hThread;       ///Mouse watch thread handle.
   HANDLE m_hDataMutex;    ///Mutex protecting access to the various settings. These are shared between the mouse watch thread and application thread.
   HANDLE m_hTerminateThreadEvt; ///Event used to terminate the thread.

   RECT m_centerRect;      ///Rectangle outside of which warp should be initiated. Desktop rect corrected with sensibility. Protected by m_hDataMutex.
   DWORD m_reWarpDelay;    ///Delay before a second warp is triggered on the same border. 0 to disable. Protected by m_hDataMutex.

   LONG m_sensibility;     ///Number of pixels from the border for warp to be triggered
   DWORD m_minDuration;    ///Minimum duration to stay on a border for warp to happen

   int m_warpVKey;         ///Optional VKey to press to enable warp. 0 to disable.
   bool m_invertMousePos;  ///Does the mouse move to the other side of the screen when switching ?
   bool m_enableWarp;      ///Is warp enable ?
};

#endif /*__MOUSEWARP_H__*/
