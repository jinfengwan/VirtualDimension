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

#ifndef __TRAYICON_H__
#define __TRAYICON_H__

#include "TrayIconsManager.h"
#include "HotkeyConfig.h"
#include "Settings.h"

class TrayIcon: public TrayIconsManager::TrayIconHandler
{
public:
   TrayIcon(HWND hWnd);
   virtual ~TrayIcon(void);

   void SetIcon(bool res);
   bool HasIcon() const              { return m_iconLoaded; }
   bool IsCloseToTray() const        { return m_closeToTray; }
   void SetCloseToTray(bool totray)  { m_closeToTray = totray; }

protected:
   void AddIcon();
   void DelIcon();

   LRESULT OnTrayIconMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   HICON GetIcon();
   char* GetText(); 

   LRESULT OnCmdClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   void OnLeftButtonDown();
   void OnContextMenu();

   HWND m_hWnd;
   bool m_iconLoaded;
   bool m_closeToTray;

	class ToggleWindowEventHandler: public PersistentHotkey<Settings::TogglePreviewWindowHotkey>
   {
   public:
      virtual void OnHotkey();
      virtual LPCSTR GetName() const									{ return "Show/hide preview window"; }
   };

   ToggleWindowEventHandler m_toggleWindowEventHandler;
};

#endif /*__TRAYICON_H__*/
