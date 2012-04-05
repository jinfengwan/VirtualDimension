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

#ifndef __HOTKEYMANAGER_H__
#define __HOTKEYMANAGER_H__

#include <map>

using namespace std;

class HotKeyManager
{
public:
   class EventHandler {
   public:
      virtual void OnHotkey() = 0;
   };

   HotKeyManager(void);
   virtual ~HotKeyManager(void);
   static HotKeyManager* GetInstance()  { return &instance; }

   void RegisterHotkey(int hotkey, EventHandler* handler);
   void UnregisterHotkey(EventHandler* handler);
   EventHandler* GetHotkeyData(int id);

protected:
   LRESULT OnHotkey(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   static HotKeyManager instance; //the singleton instance

   map<int, EventHandler*> * m_map;
};

#endif /*__HOTKEYMANAGER_H__*/
