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
#include "trayicon.h"
#include "settings.h"
#include "VirtualDimension.h"
#include "DesktopManager.h"
#include "Locale.h"

TrayIcon::TrayIcon(HWND hWnd): m_hWnd(hWnd), m_iconLoaded(false)
{
   Settings settings;

   if (settings.LoadSetting(Settings::HasTrayIcon))
      AddIcon();

   m_closeToTray = settings.LoadSetting(Settings::CloseToTray);
   vdWindow.SetSysCommandHandler(SC_CLOSE, this, &TrayIcon::OnCmdClose);
}

TrayIcon::~TrayIcon(void)
{
   Settings settings;

   settings.SaveSetting(Settings::HasTrayIcon, m_iconLoaded);
   settings.SaveSetting(Settings::CloseToTray, m_closeToTray);

   DelIcon();
}

void TrayIcon::AddIcon()
{
   if (m_iconLoaded)
      return;

   m_iconLoaded = trayManager->AddIcon(this);
}

void TrayIcon::DelIcon()
{
   if (!m_iconLoaded)
      return;

   m_iconLoaded = false;
   trayManager->DelIcon(this);

   /* make sure the window can be seen if we remove the tray icon */
   ShowWindow(m_hWnd, SW_SHOW);
}

void TrayIcon::SetIcon(bool res)
{
   if (res && !m_iconLoaded)
      AddIcon();
   else if (!res && m_iconLoaded)
      DelIcon();
}

LRESULT TrayIcon::OnTrayIconMessage(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   switch(lParam)
   {
   case WM_RBUTTONDOWN:
   case WM_CONTEXTMENU:
      OnContextMenu();
      break;

   case WM_LBUTTONDOWN:
      OnLeftButtonDown();
      break;
   }

   return 0;
}

void TrayIcon::OnContextMenu()
{
   HMENU hMenu, hmenuTrackPopup;
   POINT pt;
   HRESULT res;
   int i;
   Desktop * desk;

   //Get the "base" menu
   hMenu = Locale::GetInstance().LoadMenu(IDC_VIRTUALDIMENSION);
   if (hMenu == NULL)
      return;
   hmenuTrackPopup = GetSubMenu(hMenu, 0);

   //Append the list of desktops
   InsertMenu(hmenuTrackPopup, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

   i = 0;
   for( desk = deskMan->GetFirstDesktop();
        desk != NULL;
        desk = deskMan->GetNextDesktop())
   {
      MENUITEMINFO mii;

      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_STATE | MIIM_STRING | MIIM_ID | MIIM_DATA;
      if (desk->IsActive())
         mii.fState = MFS_CHECKED;
      else
         mii.fState = MFS_UNCHECKED;
      mii.dwItemData = (DWORD)desk;
      mii.dwTypeData = desk->GetText();
      mii.cch = strlen(mii.dwTypeData);
      mii.wID = WM_USER + i++;
      InsertMenuItem(hmenuTrackPopup, 0, TRUE, &mii);
   }

   //And show the menu
   GetCursorPos(&pt);
   SetForegroundWindow(m_hWnd);
   res = TrackPopupMenu(hmenuTrackPopup, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL);

   //Process the resulting message
   if (res >= WM_USER)
   {
      Desktop * desk;
      MENUITEMINFO mii;

      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_DATA;
      GetMenuItemInfo(hmenuTrackPopup, res, FALSE, &mii);
      desk = (Desktop*)mii.dwItemData;

      deskMan->SwitchToDesktop(desk);
      InvalidateRect(m_hWnd, NULL, TRUE);
   }
   else
      PostMessage(m_hWnd, WM_COMMAND, res, 0);

   //Do not forget to destroy the menu
   DestroyMenu(hMenu);
}

void TrayIcon::OnLeftButtonDown()
{
   if (IsWindowVisible(m_hWnd))
      ShowWindow(m_hWnd, SW_HIDE);
   else
   {
      SetForegroundWindow(m_hWnd);
      ShowWindow(m_hWnd, SW_SHOW);
   }
}

LRESULT TrayIcon::OnCmdClose(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   if (m_closeToTray && m_iconLoaded && (HIWORD(lParam) != 0) && (HIWORD(lParam) != (WORD)-1))
      ShowWindow(m_hWnd, SW_HIDE);
   else
      PostMessage(hWnd, WM_COMMAND, IDM_EXIT, 0);

   return 0;
}

HICON TrayIcon::GetIcon()
{
   return LoadIcon(vdWindow, (LPCSTR)IDI_VIRTUALDIMENSION); 
}

char* TrayIcon::GetText()
{
   if (deskMan && deskMan->GetCurrentDesktop())
      return deskMan->GetCurrentDesktop()->GetText();
   else
      return "";
}

void TrayIcon::ToggleWindowEventHandler::OnHotkey()
{
   if (!IsWindowVisible(vdWindow))
      ShowWindow(vdWindow, SW_SHOW);
   else if (GetForegroundWindow() == (HWND)vdWindow)
      ShowWindow(vdWindow, SW_HIDE);
   else
      SetForegroundWindow(vdWindow);
}
