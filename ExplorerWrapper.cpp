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
#include "ExplorerWrapper.h"

#include "TrayIconsManager.h"
#include "DesktopManager.h"

ExplorerWrapper * explorerWrapper;

ExplorerWrapper::ExplorerWrapper(FastWindow * wnd)
{
   UINT uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
   wnd->SetCommandHandler(uTaskbarRestart, this, &ExplorerWrapper::OnTaskbarRestart);

   BindTaskbar();

	m_pActiveDesktop = NULL;
   BindActiveDesktop();
}

ExplorerWrapper::~ExplorerWrapper(void)
{
#ifdef HIDEWINDOW_COMINTERFACE
   if (m_tasklist)
      m_tasklist->Release();
#endif
   if (m_pActiveDesktop)
      m_pActiveDesktop->Release();
}

void ExplorerWrapper::BindTaskbar()
{
#ifdef HIDEWINDOW_COMINTERFACE

   if (CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList, (LPVOID*)&m_tasklist) == S_OK)
      m_tasklist->HrInit();
   else
      m_tasklist = NULL;

#else

   m_ShellhookMsg = RegisterWindowMessage("SHELLHOOK");

   HWND hwndTray = FindWindowEx(NULL, NULL, "Shell_TrayWnd", NULL);
   HWND hwndBar = FindWindowEx(hwndTray, NULL, "ReBarWindow32", NULL );

   // Maybe "RebarWindow32" is not a child to "Shell_TrayWnd", then try this
   if( hwndBar == NULL )
      hwndBar = hwndTray;

   m_hWndTasklist = FindWindowEx(hwndBar, NULL, "MSTaskSwWClass", NULL);

#endif
}

bool ExplorerWrapper::BindActiveDesktop()
{
	if (m_pActiveDesktop)
		m_pActiveDesktop->Release();
   if (CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (LPVOID*)&m_pActiveDesktop) != S_OK)
      m_pActiveDesktop = NULL;
	return m_pActiveDesktop != NULL;
}

LRESULT ExplorerWrapper::OnTaskbarRestart(HWND /*hWnd*/, UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   //Refresh tray icons
   trayManager->RefreshIcons();

   //Ensure we got the correct interface for showing/hiding icons from the taskbar
#ifdef HIDEWINDOW_COMINTERFACE
   m_tasklist->Release();  //release the interface before re-binding it...
#endif
   BindTaskbar();
   BindActiveDesktop();

   //Restore wallpaper
   deskMan->GetCurrentDesktop()->RefreshWallpaper();

   return 0;
}
