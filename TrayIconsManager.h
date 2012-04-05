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

#ifndef __TRAYICONSMANAGER_H__
#define __TRAYICONSMANAGER_H__

#include <set>

using namespace std;

class TrayIconsManager
{
public:
   class TrayIconHandler
   {
   public:
      TrayIconHandler(): m_callbackMessage(0) { return; }
      void Update();
      void DisplayBalloon(LPTSTR message = NULL, LPTSTR title = NULL, UINT timeout = 0 /*millisecond*/, DWORD flags = 0/*NIIF_* notify icon flags*/);
   protected:
	   virtual LRESULT OnTrayIconMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
      virtual HICON GetIcon() = 0;
      virtual char* GetText() = 0;
   private:
	   UINT m_callbackMessage;
	   friend class TrayIconsManager;
   };

	TrayIconsManager();
	~TrayIconsManager();

	bool AddIcon(TrayIconHandler* handler);
	bool DelIcon(TrayIconHandler* handler);

   void RefreshIcons();

protected:
	set<TrayIconHandler *> m_registered_handlers;
   static UINT s_nextCallbackMessage;
};

extern TrayIconsManager * trayManager;

#endif /*__TRAYICONSMANAGER_H__*/
