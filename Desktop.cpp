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
#include "Desktop.h"
#include <string>
#include <Shellapi.h>
#include <assert.h>
#include "WindowsManager.h"
#include "DesktopManager.h"
#include "WindowsList.h"
#include "VirtualDimension.h"
#include "PlatformHelper.h"
#include "Locale.h"

Desktop::Desktop(int i)
{
	char * basename;

   m_active = false;
   m_hotkey = 0;
   m_rect.bottom = m_rect.left = m_rect.right = m_rect.top = 0;
   strcpy(m_wallpaperFile, DESKTOP_WALLPAPER_DEFAULT);
   m_bkColor = GetSysColor(COLOR_DESKTOP);

   m_wallpaper.SetImage(FormatWallpaper(m_wallpaperFile));
   m_wallpaper.SetColor(m_bkColor);

	locGetString(basename, IDS_DESKTOP_BASENAME);
   sprintf(m_name, "%s%i", basename, i);
}

Desktop::Desktop(Settings::Desktop * desktop)
{
   desktop->GetName(m_name, sizeof(m_name));
   desktop->LoadSetting(Settings::Desktop::DeskWallpaper, m_wallpaperFile, sizeof(m_wallpaperFile));
   m_index = desktop->LoadSetting(Settings::Desktop::DeskIndex);
   m_hotkey = desktop->LoadSetting(Settings::Desktop::DeskHotkey);
   m_bkColor = desktop->LoadSetting(Settings::Desktop::BackgroundColor);

   m_wallpaper.SetImage(FormatWallpaper(m_wallpaperFile));
   m_wallpaper.SetColor(m_bkColor);

   m_active = false;

   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->RegisterHotkey(m_hotkey, this);
}

Desktop::~Desktop(void)
{
   WindowsManager::Iterator it;

   //Show the hidden windows, if any
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;

      if (win->IsOnDesk(this))
         win->ShowWindow();
   }

   //Unregister the hotkey
   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->UnregisterHotkey(this);

   //Remove the tooltip tool
   tooltip->UnsetTool(this);
}

HMENU Desktop::BuildMenu()
{
   WindowsManager::Iterator it;
   HMENU hMenu;
   MENUITEMINFO mii;
   MENUINFO mi;

   //Create the menu
   hMenu = CreatePopupMenu();

   //Set its style
   mi.cbSize = sizeof(MENUINFO);
   mi.fMask = MIM_STYLE;
   mi.dwStyle = MNS_CHECKORBMP;
   PlatformHelper::SetMenuInfo(hMenu, &mi);

   //Add the menu items
   mii.cbSize = sizeof(mii);
   mii.fMask = MIIM_STRING | MIIM_ID | MIIM_DATA | MIIM_BITMAP;

   for(it = winMan->GetIterator(); it; it++)
   {
      TCHAR buffer[50];
      DWORD res;
      Window * win = it;

      if (!win->IsOnDesk(this))
         continue;

      SendMessageTimeout(*win, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer, SMTO_ABORTIFHUNG, 50, &res);

      mii.dwItemData = (DWORD)win->GetIcon();
      mii.dwTypeData = buffer;
      mii.cch = strlen(buffer);
      mii.wID = WM_USER+(int)win;	//this is not really clean, and could theoretically overflow...  no real problem, though...
      mii.hbmpItem = HBMMENU_CALLBACK;

		assert(mii.wID > WM_USER);

      InsertMenuItem(hMenu, (UINT)-1, TRUE, &mii);
   }

   return hMenu;
}

void Desktop::OnMenuItemSelected(HMENU /*menu*/, int cmdId)
{
   Window * win;

   win = (Window *)(cmdId-WM_USER);
   win->Activate();
}

void Desktop::resize(LPRECT rect)
{
   m_rect.left = rect->left + 2;
   m_rect.top = rect->top + 2;
   m_rect.right = rect->right - 2;
   m_rect.bottom = rect->bottom - 2;

   UpdateLayout();
}

void Desktop::UpdateLayout()
{
   tooltip->SetTool(this);

   WindowsManager::Iterator it;
   int x, y;

   x = m_rect.left;
   y = m_rect.top;
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;
      RECT rect;

      if (!win->IsOnDesk(this))
         continue;

      rect.left = x;
      rect.top = y;
      rect.right = x + 16;
      rect.bottom = y + 16;

      tooltip->SetTool(win, &rect);

      x += 16;
      if (x > m_rect.right-15)
      {
         x = m_rect.left;
         y += 16;
      }
   }

   vdWindow.Refresh();
}

void Desktop::Draw(HDC hDc)
{
   char buffer[20];

   //Print desktop name in the middle
   sprintf(buffer, "%.19s", m_name);
   SetBkMode(hDc, TRANSPARENT);
   DrawText(hDc, buffer, -1, &m_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

   //Draw a frame around the desktop
   FrameRect(hDc, &m_rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

   //Draw icons for each window
   WindowsManager::Iterator it;
   list<Window*> obsoleteWindowsList;
   int x, y;

   x = m_rect.left;
   y = m_rect.top;
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;
      HICON hIcon;

      //Mark obsolete windows for removal
      if (!win->CheckExists())
      {
         obsoleteWindowsList.push_front(win);
         continue;
      }

      //Skip windows not present on this desk
      if (!win->IsOnDesk(this))
         continue;

      //Draw the window's icon
      hIcon = win->GetIcon();
      DrawIconEx(hDc, x, y, hIcon, 16, 16, 0, NULL, DI_NORMAL);

      x += 16;
      if (x > m_rect.right-16)
      {
         x = m_rect.left;
         y += 16;
      }
   }

   //Remove windows flagged as obsolete
   for(list<Window*>::iterator i = obsoleteWindowsList.begin();
       i != obsoleteWindowsList.end();
       i++)
      winMan->RemoveWindow(*i);
   obsoleteWindowsList.clear();
}

/** Get a pointer to the window represented at some position.
 * This function returns a pointer to the window object that is represented at the specified
 * position, if any. If there is no such window, it returns NULL.
 * In addition, one should make sure the point is in the desktop's rectangle.
 *
 * @param X Horizontal position of the cursor
 * @param Y Vertical position of the cursor
 */
Window* Desktop::GetWindowFromPoint(int X, int Y)
{
   WindowsManager::Iterator it;
   int index;

   index = ((X - m_rect.left) / 16) +
           ((m_rect.right-m_rect.left) / 16) * ((Y - m_rect.top) / 16);

   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;

      if (!win->IsOnDesk(this))
         continue;

      if (index == 0)
         return win;
      index --;
   }

   return NULL;
}

void Desktop::Rename(char * name)
{
   Settings settings;
   Settings::Desktop desktop(&settings, m_name);

   /* Remove the desktop from registry */
   desktop.Destroy();

   /* copy the new name */
   strncpy(m_name, name, sizeof(m_name));
}

void Desktop::Remove()
{
   Settings settings;
   Settings::Desktop desktop(&settings, m_name);

   /* Remove the desktop from registry */
   desktop.Destroy();

   /* Move all windows present only on this desktop to the current desk */
   Desktop * curDesk;
   WindowsManager::Iterator it;

   curDesk = deskMan->GetCurrentDesktop();
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;

      if ( (win->IsOnDesk(this)) &&
          !(win->IsOnDesk(NULL)) )
         win->MoveToDesktop(curDesk);
   }
}

void Desktop::Save()
{
   Settings settings;
   Settings::Desktop desktop(&settings, m_name);

   desktop.SaveSetting(Settings::Desktop::DeskWallpaper, m_wallpaperFile);
   desktop.SaveSetting(Settings::Desktop::DeskIndex, m_index);
   desktop.SaveSetting(Settings::Desktop::DeskHotkey, m_hotkey);
   desktop.SaveSetting(Settings::Desktop::BackgroundColor, m_bkColor);
}

void Desktop::ShowWindowWorkerProc(void * lpParam)
{
   Window * win = (Window *)lpParam;

   win->SetSwitching(true);

   if (win->IsInTray())
      trayManager->AddIcon(win);
   else
   {
      win->ShowWindow();
   }

   win->SetSwitching(false);
}

void Desktop::HideWindowWorkerProc(void * lpParam)
{
   Window * win = (Window*)lpParam;

   win->SetSwitching(true);

   if (win->IsInTray())
      trayManager->DelIcon(win);
   else
      win->HideWindow();

   win->SetSwitching(false);
}

BOOL CALLBACK Desktop::ActivateTopWindowProc( HWND hWnd, LPARAM lParam ) {
	HWND tmpOwner = hWnd;
	HWND OwnerWindow = NULL;
	Window * win;

	if(!(GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE) || (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
		return TRUE;

	while((tmpOwner = ::GetWindow(tmpOwner, GW_OWNER)) != NULL)
		OwnerWindow = tmpOwner;
	if(OwnerWindow) {
		if((GetWindowLong(OwnerWindow, GWL_STYLE) & WS_VISIBLE) && !(GetWindowLong(OwnerWindow, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
			win = winMan->GetWindow(OwnerWindow);
		else
			return TRUE;
	}
	else
		win = winMan->GetWindow(hWnd);
	if(win && win->IsOnDesk((Desktop *)lParam)) {
		SetForegroundWindow(hWnd);
		return FALSE;
	}
	return TRUE;
}

void Desktop::Activate(void)
{
	bool WindowsOnThisDesk = false;

   WindowsManager::Iterator it;

   m_active = true;

   /* Set the wallpaper */
   m_wallpaper.Activate();

   //This helps to ensure we can set the foreground window (theorical), and ensures we got the captions
   //painted correctly (only one "active" window).
	SetForegroundWindow(vdWindow);

	// Show/hide the windows
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;

      //Ignore obsolete windows
      if (!win->CheckExists())
         continue;

		if (win->IsMoving())
		{
			win->MoveToDesktop(this);
		}
      else if (win->IsOnDesk(this))
      {
      	HANDLE event;
      	win->UnFlashWindow();
      	if (!m_taskPool.UpdateJob(HideWindowWorkerProc, win, ShowWindowWorkerProc, win, &event))
            m_taskPool.QueueJob(ShowWindowWorkerProc, win, &event);
         WaitForSingleObject(event, 2000);
			WindowsOnThisDesk = true;
      }
		else if(!win->IsHidden())
      {
         if (!m_taskPool.UpdateJob(ShowWindowWorkerProc, win, HideWindowWorkerProc, win))
            m_taskPool.QueueJob(HideWindowWorkerProc, win);
      }
   }
	if(WindowsOnThisDesk) { //don't bother with all this if empty desktop
		EnumWindows(ActivateTopWindowProc, (LPARAM)this);
	}
}

void Desktop::Desactivate(void)
{
   m_active = false;
}

void Desktop::SetHotkey(int hotkey)
{
   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->UnregisterHotkey(this);

   m_hotkey = hotkey;

   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->RegisterHotkey(m_hotkey, this);
}

void Desktop::OnHotkey()
{
   deskMan->SwitchToDesktop(this);
}

LPTSTR Desktop::FormatWallpaper(LPTSTR fileName)
{
   LPTSTR res;

   if (*fileName == 0)
   {
      strcpy(fileName, DESKTOP_WALLPAPER_DEFAULT);
      res = "";
   }
   else if (stricmp(fileName, DESKTOP_WALLPAPER_NONE) == 0)
      res = NULL;
   else if (stricmp(fileName, DESKTOP_WALLPAPER_DEFAULT) == 0)
      res = "";
   else
      res = fileName;

   return res;
}

void Desktop::SetWallpaper(LPCTSTR fileName)
{
   strncpy(m_wallpaperFile, fileName, sizeof(m_wallpaperFile)/sizeof(TCHAR));
   m_wallpaper.SetImage(FormatWallpaper(m_wallpaperFile));
}

void Desktop::SetBackgroundColor(COLORREF col)
{
   if (col == m_bkColor)
      return;

   m_bkColor = col;

   m_wallpaper.SetColor(m_bkColor);
}

bool Desktop::deskOrder(Desktop * first, Desktop * second)
{
   return first->m_index < second->m_index;
}
