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
#include "BalloonNotif.h"
#include "VirtualDimension.h"
#include "PlatformHelper.h"

// Use an empiric 'random' timer id, to minimize interference with timers used by tooltip control
#define BALLOON_NOTIFICATION_TIMEOUT   0xa341c85e

BalloonNotification msgManager;

BalloonNotification::BalloonNotification(void): m_tooltipWndProc(NULL), m_tooltipWndExtra(-1), m_tooltipClass(NULL)
{
	WNDCLASSEX cls;
	INITCOMMONCONTROLSEX cmnctrl;

   //Ensure tooltip class is loaded
   cmnctrl.dwSize = sizeof(cmnctrl);
   cmnctrl.dwICC = ICC_TAB_CLASSES;
   InitCommonControlsEx(&cmnctrl);

   //Get tooltip's class info
   cls.cbSize = sizeof(WNDCLASSEX);
	if (GetClassInfoEx(NULL, TOOLTIPS_CLASS, &cls))
	{
		// Subclass
		m_tooltipWndProc = cls.lpfnWndProc;
		cls.lpfnWndProc = &MyTooltipWndProc;

		// Add some storage space
		m_tooltipWndExtra = cls.cbWndExtra;
		cls.cbWndExtra += 2*sizeof(int);

		// Register the new class
		cls.hInstance = (HINSTANCE)GetModuleHandle(NULL);
		cls.lpszClassName = "Balloon Notification Tooltips";
		//cls.style |= CS_GLOBALCLASS;
		m_tooltipClass = MAKEINTATOM(RegisterClassEx(&cls));
	}
   
   if (m_tooltipClass == NULL)
   {
      // Failed to get class info -> use standard class
      m_tooltipClass = TOOLTIPS_CLASS;
   }
}

BalloonNotification::~BalloonNotification(void)
{
	UnregisterClass(m_tooltipClass, GetModuleHandle(NULL));
}

BalloonNotification::Message BalloonNotification::Add(LPCTSTR text, LPCTSTR title, int icon, 
                                                      BalloonNotification::ClickCb cb, int data, DWORD timeout)
{
   HWND hwndToolTips = CreateWindowEx(WS_EX_TOPMOST, m_tooltipClass, NULL,
                         WS_POPUP | TTS_NOPREFIX | TTS_BALLOON | ((cb) ? TTS_CLOSE : 0),
                         0, 0, 0, 0,
                         NULL, NULL, vdWindow, NULL);

   if (hwndToolTips)
   {
   	// Store the callback information
   	if (m_tooltipWndExtra >= 0)
      {
         SetWindowLongPtr(hwndToolTips, m_tooltipWndExtra, (LONG_PTR)cb);
         SetWindowLongPtr(hwndToolTips, m_tooltipWndExtra+sizeof(int), (LONG_PTR)data);
      }

      // Create a tool
      TOOLINFO ti;
      ti.cbSize = sizeof(ti);
      ti.uFlags = TTF_CENTERTIP | TTF_TRACK;
      ti.hwnd = vdWindow;
      ti.uId = 0;
      ti.hinst = NULL;
      ti.lpszText = (LPSTR)text;
      
      memset(&ti.rect, 0, sizeof(ti.rect));
      SendMessage(hwndToolTips, TTM_ADDTOOL, 0, (LPARAM) &ti);

      // Set the title, if any
      if (title)
      	SendMessage(hwndToolTips, TTM_SETTITLE, (WPARAM)icon, (LPARAM)title);  

      // Position and display the tooltip
      SendMessage(hwndToolTips, TTM_TRACKPOSITION, 0, 0);
      SendMessage(hwndToolTips, TTM_TRACKACTIVATE, TRUE, (LPARAM) &ti);

      // Setup timeout for automatically disappearing
      if (timeout)
         SetTimer(hwndToolTips, BALLOON_NOTIFICATION_TIMEOUT, timeout, NULL);
   }

   return hwndToolTips;
}

void BalloonNotification::Remove(BalloonNotification::Message tip)
{
	DestroyWindow(tip);
}

LRESULT BalloonNotification::MyTooltipWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = 0;
	ClickCb cb;

   switch(Msg)
   {
   case WM_LBUTTONDOWN:
      cb = (ClickCb)GetWindowLongPtr(hWnd, msgManager.m_tooltipWndExtra);
      if (cb)
         (*cb)(hWnd, GetWindowLongPtr(hWnd, msgManager.m_tooltipWndExtra+sizeof(int)));
      DestroyWindow(hWnd);
      break;

   case WM_RBUTTONDOWN:
	  DestroyWindow(hWnd);
      break;

   case WM_TIMER:
      if (wParam == BALLOON_NOTIFICATION_TIMEOUT)
         DestroyWindow(hWnd);
      else
         res = CallWindowProc(msgManager.m_tooltipWndProc, hWnd, Msg, wParam, lParam);
      break;

   default:
      res = CallWindowProc(msgManager.m_tooltipWndProc, hWnd, Msg, wParam, lParam);
      break;
   }

   return res;
}
