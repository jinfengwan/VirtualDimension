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
#include "window.h"
#include "VirtualDimension.h"
#include "movewindow.h"
#include <Shellapi.h>
#include "PlatformHelper.h"
#include "window.h"
#include "ExplorerWrapper.h"
#include "DesktopManager.h"
#include "WindowsManager.h"
#include "SharedMenuBuffer.h"
#include "HookDLL.h"
#include "Locale.h"

HINSTANCE HookWindow(HWND hWnd, DWORD dwProcessId, int data);
bool UnHookWindow(HINSTANCE hInstance, DWORD dwProcessId, HWND hWnd);

HidingMethodHide       Window::s_hider_method;
HidingMethodMinimize   Window::s_minimizer_method;
HidingMethodMove       Window::s_mover_method;

HidingMethod* Window::s_hiding_methods[] =
{
   &s_hider_method,
   &s_minimizer_method,
   &s_mover_method
};

const ATOM Window::s_VDPropertyTag = GlobalAddAtom("ViRtUaL DiMeNsIoN rocks !");


Window::Window(HWND hWnd): AlwaysOnTop(hWnd), m_hWnd(hWnd), m_hOwnedWnd(GetOwnedWindow(hWnd)),
                           m_MinToTray(false), m_style(0), m_transp(m_hOwnedWnd), m_transpLevel(128),
                           m_autoSaveSettings(false), m_autosize(false), m_autopos(false), m_autodesk(false),
                            m_hIcon(NULL), m_hDefaulIcon(NULL), m_BallonMsg(NULL), m_HookDllHandle(NULL),
                           m_switching(false), m_moving(false), m_hidden(false)
{
   Settings s;
   Settings::Window settings(&s);
   unsigned int method;

   //Try to see if there are some special settings for this window
   GetClassName(m_hWnd, m_className, sizeof(m_className)/sizeof(TCHAR));
   OpenSettings(settings, false);

   //Setup the hiding method to use (try to restore from tag, if present)
   m_hHideMutex = CreateMutex(NULL, FALSE, NULL);
   if (!HasTag(hWnd))
   {
      TCHAR filename[MAX_PATH];
      PlatformHelper::GetWindowFileName(m_hWnd, filename, MAX_PATH);
      method = s.LoadHidingMethod(filename);
   }
   else
      method = GetTag(hWnd);
   if (method > sizeof(s_hiding_methods)/sizeof(*s_hiding_methods))
      method = 0;
   m_hidingMethod = s_hiding_methods[method];
   m_hidingMethod->Attach(this);

   //Tag the window, to remember the window was managed by VD (in case of crash). It also tracks the hidding method used
   SetTag(hWnd, method);

   //Load settings for this window
   m_desk = settings.LoadSetting(Settings::Window::OnAllDesktops) ? NULL : deskMan->GetCurrentDesktop();

   SetMinimizeToTray(settings.LoadSetting(Settings::Window::MinimizeToTray));
   SetAlwaysOnTop(settings.LoadSetting(Settings::Window::AlwaysOnTop));

   SetTransparencyLevel(settings.LoadSetting(Settings::Window::TransparencyLevel));
   SetTransparent(settings.LoadSetting(Settings::Window::EnableTransparency));

   m_autoSaveSettings = settings.LoadSetting(Settings::Window::AutoSaveSettings);
   m_autosize = settings.LoadSetting(Settings::Window::AutoSetSize);
   m_autopos = settings.LoadSetting(Settings::Window::AutoSetPos);
   m_autodesk = settings.LoadSetting(Settings::Window::AutoSetDesk);

   //Auto-switch desktop
   if (m_autodesk && !IsOnAllDesktops())
   {
      Desktop * desk = deskMan->GetDesktop(settings.LoadSetting(Settings::Window::DesktopIndex));
      if (desk)
         //Move the window to its associated desk
         MoveToDesktop(desk);
      else if (!m_autoSaveSettings)
         //Disable auto-move window to desktop (keep enabled if auto-saving settings)
         settings.SaveSetting(Settings::Window::AutoSetDesk, m_autodesk = false);
   }

   //Delay-update
   winMan->ScheduleDelayedUpdate(this);
}

Window::~Window(void)
{
   winMan->CancelDelayedUpdate(this);

   UnHook();

   if (m_hDefaulIcon)
      DestroyIcon(m_hDefaulIcon);

   if (m_autoSaveSettings)
      SaveSettings();

   if (m_hHideMutex)
      CloseHandle(m_hHideMutex);

   m_hidingMethod->Detach(this);

   //Tag is not needed anymore
   RemTag(m_hWnd);
}

/** Delay update callback.
 * This method is called as the final step in the delayed-update process. This process is used to
 * fix some issues when VD is called too early after a window has been created. Indeed, sometime
 * the owned window is not ready, thus the user would not get the VD menu, and many features would
 * not work (always on top, transparency...).
 * This function is thus called some time after the constructor (provided a call to
 * WindowManager::ScheduleDelayedUpdate() is made). It checks if the owned window has changed,
 * updates various parameters consequently, and in any case performs auto-size/position and shell
 * integration.
 */
void Window::OnDelayUpdate()
{
   Settings s;
   Settings::Window settings(&s);
   RECT rect;
   HWND hOwnedWnd = GetOwnedWindow(m_hWnd);

   if (hOwnedWnd != m_hOwnedWnd)
   {
      m_hOwnedWnd = hOwnedWnd;
      m_transp.SetWindow(m_hOwnedWnd);
   }

   //Auto-size/position
   OpenSettings(settings, false);
   if ( (m_autosize || m_autopos) && settings.LoadSetting(Settings::Window::WindowPosition, &rect) )
      SetWindowPos(m_hOwnedWnd, 0,
      rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
      SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS |
      (m_autopos?0:SWP_NOMOVE) | (m_autosize?0:SWP_NOSIZE));

   //Shell integration
   TCHAR filename[MAX_PATH];
   PlatformHelper::GetWindowFileName(m_hWnd, filename, MAX_PATH);
   if (winMan->IsIntegrateWithShell() && !s.LoadDisableShellIntegration(filename))
      Hook();
}

void Window::MoveToDesktop(Desktop * desk)
{
   Desktop * oldDesk;

   if (desk == m_desk)
      return;

   oldDesk = m_desk;
   m_desk = desk;

   if (deskMan->GetCurrentDesktop() == desk)
      UnFlashWindow();

   if (IsOnDesk(deskMan->GetCurrentDesktop()))
   {
      if (IsInTray())
         trayManager->AddIcon(this);
      else
         ShowWindow();
   }
   else
   {
      if (IsInTray())
         trayManager->DelIcon(this);
      else
         HideWindow();
   }

   if (IsOnDesk(NULL))  //on all desktops
      deskMan->UpdateLayout();
   else
   {
      if (oldDesk != NULL)
      {
         oldDesk->UpdateLayout();
         m_desk->UpdateLayout();
      }
      else
         deskMan->UpdateLayout();
   }
}

bool Window::IsOnCurrentDesk() const
{
   return IsOnDesk(deskMan->GetCurrentDesktop());
}

HICON Window::GetIcon(void)
{
   if (GetObjectType(m_hIcon))
      return m_hIcon;

   m_hIcon = NULL;

   //Get normal icon
	if (SendMessageTimeout(m_hWnd, WM_GETICON, ICON_SMALL, 0, SMTO_ABORTIFHUNG, 100, (LPDWORD)&m_hIcon) &&
       m_hIcon)
      return m_hIcon;

   //Get drag icon
   if (SendMessageTimeout(m_hWnd, WM_QUERYDRAGICON, 0, 0, SMTO_ABORTIFHUNG, 100, (LPDWORD)&m_hIcon) &&
       m_hIcon)
      return m_hIcon;

   //Get class icon
   m_hIcon = (HICON) GetClassLong( m_hWnd, GCL_HICONSM );
   if (m_hIcon)
      return m_hIcon;

   //Return default icon
   if (m_hDefaulIcon)
      return m_hDefaulIcon;

   //No default icon yet: get it from application file, or use generic default icon
   TCHAR lpFileName[256];
   PlatformHelper::GetWindowFileName(m_hWnd, lpFileName, 256);
   if (!ExtractIconEx(lpFileName, 0, NULL, &m_hDefaulIcon, 1))
      m_hDefaulIcon = (HICON) LoadImage(vdWindow, MAKEINTRESOURCE(IDI_DEFAPP_SMALL), IMAGE_ICON, 16, 16, LR_SHARED);

   return m_hDefaulIcon;
}

void Window::InsertMenuItem(HMENU menu, bool checked, HANDLE bmp, UINT id, UINT uIdStr)
{
   MENUITEMINFO mii;

   mii.cbSize = sizeof(MENUITEMINFO);
   mii.fMask = MIIM_DATA | MIIM_BITMAP | MIIM_ID | MIIM_STRING | MIIM_STATE;
   mii.hbmpItem = (int)bmp <= 11 ? (HBITMAP)bmp : HBMMENU_CALLBACK;
   mii.dwItemData = (ULONG_PTR)bmp;
   mii.wID = id;
	locGetString(mii.dwTypeData, uIdStr);
   mii.fState = checked ? MFS_CHECKED : MFS_UNCHECKED;
   ::InsertMenuItem(menu, (UINT)-1, TRUE, &mii);
}

void Window::InsertMenuInfo(SharedMenuBuffer& menuinfo, UINT id, UINT uIdStr, bool checked)
{
	char * text;
	locGetString(text, uIdStr);
	menuinfo.InsertMenu(id, text, checked);
}

HANDLE Window::LoadBmpRes(int id)
{
   return LoadImage(vdWindow, MAKEINTRESOURCE(id), IMAGE_ICON, 16, 16, LR_SHARED);
}

HMENU Window::BuildMenu()
{
   HMENU hMenu;
   MENUINFO mi;

   //Create the menu
   hMenu = CreatePopupMenu();

   //Set its style
   mi.cbSize = sizeof(MENUINFO);
   mi.fMask = MIM_STYLE;
   mi.dwStyle = MNS_CHECKORBMP;
   PlatformHelper::SetMenuInfo(hMenu, &mi);

   //Now add the items
   InsertMenuItem(hMenu, IsAlwaysOnTop(), NULL, VDM_TOGGLEONTOP, IDS_MENU_ALWAYSONTOP);
   InsertMenuItem(hMenu, IsMinimizeToTray(), NULL, VDM_TOGGLEMINIMIZETOTRAY, IDS_MENU_MINTOTRAY);
   if (m_transp.IsTransparencySupported())
      InsertMenuItem(hMenu, m_transp.GetTransparencyLevel() != 255, NULL, VDM_TOGGLETRANSPARENCY, IDS_MENU_TRANSPARENT);
   AppendMenu(hMenu, MF_SEPARATOR, 0, 0);

   InsertMenuItem(hMenu, IsOnAllDesktops(), NULL, VDM_TOGGLEALLDESKTOPS, IDS_MENU_ONALLDESKTOPS);
   InsertMenuItem(hMenu, FALSE, NULL, VDM_MOVEWINDOW, IDS_MENU_CHANGEDESKTOP);
   AppendMenu(hMenu, MF_SEPARATOR, 0, 0);

   InsertMenuItem(hMenu, false, HBMMENU_POPUP_RESTORE, VDM_ACTIVATEWINDOW, IDS_MENU_ACTIVATEWND);
   if (IsIconic() || IsZoomed(m_hWnd))
      InsertMenuItem(hMenu, false, HBMMENU_POPUP_RESTORE, VDM_RESTORE, IDS_MENU_RESTOREWND);
   if (!IsIconic())
      InsertMenuItem(hMenu, false, HBMMENU_POPUP_MINIMIZE, VDM_MINIMIZE, IDS_MENU_MINIMIZEWND);
   if (!IsZoomed(m_hWnd))
   {
      InsertMenuItem(hMenu, false, HBMMENU_POPUP_MAXIMIZE, VDM_MAXIMIZE, IDS_MENU_MAXIMIZEWND);
      InsertMenuItem(hMenu, false, LoadBmpRes(IDI_MAXIMIZE_VERT), VDM_MAXIMIZEHEIGHT, IDS_MENU_MAXHEIGHTWND);
      InsertMenuItem(hMenu, false, LoadBmpRes(IDI_MAXIMIZE_HORIZ), VDM_MAXIMIZEWIDTH, IDS_MENU_MAXWIDTHWND);
   }
   InsertMenuItem(hMenu, false, HBMMENU_POPUP_CLOSE, VDM_CLOSE, IDS_MENU_CLOSEWND);
   InsertMenuItem(hMenu, false, LoadBmpRes(IDI_KILL), VDM_KILL, IDS_MENU_KILLWND);
   AppendMenu(hMenu, MF_SEPARATOR, 0, 0);

   InsertMenuItem(hMenu, false, NULL, VDM_PROPERTIES, IDS_MENU_WNDPROPERTIES);

   return hMenu;
}

bool Window::PrepareSysMenu(HANDLE filemapping)
{
   SharedMenuBuffer menuinfo(m_dwProcessId, filemapping);

   InsertMenuInfo(menuinfo, VDM_TOGGLEONTOP, IDS_MENU_ALWAYSONTOP, IsAlwaysOnTop());
   InsertMenuInfo(menuinfo, VDM_TOGGLEMINIMIZETOTRAY, IDS_MENU_MINTOTRAY, IsMinimizeToTray());
   if (m_transp.IsTransparencySupported())
      InsertMenuInfo(menuinfo, VDM_TOGGLETRANSPARENCY, IDS_MENU_TRANSPARENT, IsTransparent());
   menuinfo.InsertSeparator();
   InsertMenuInfo(menuinfo, VDM_TOGGLEALLDESKTOPS, IDS_MENU_ONALLDESKTOPS, IsOnAllDesktops());
   Desktop * desk = deskMan->GetFirstDesktop();
   int i = 0;
   while(desk != NULL && menuinfo.InsertMenu(VDM_MOVETODESK+i++, desk->GetText(), GetDesk()==desk))
      desk = deskMan->GetNextDesktop();
   menuinfo.InsertSeparator();
   InsertMenuInfo(menuinfo, VDM_MOVEWINDOW, IDS_MENU_CHANGEDESKTOP, false);
   InsertMenuInfo(menuinfo, VDM_PROPERTIES, IDS_MENU_WNDPROPERTIES, false);

   return true;
}

void Window::OnMenuItemSelected(HMENU /*menu*/, int cmdId)
{
   switch(cmdId)
   {
   case VDM_ACTIVATEWINDOW:
      Activate();
      break;

   case VDM_TOGGLEONTOP:
      ToggleOnTop();
      break;

   case VDM_TOGGLEMINIMIZETOTRAY:
      ToggleMinimizeToTray();
      break;

   case VDM_TOGGLETRANSPARENCY:
      ToggleTransparent();
      break;

   case VDM_TOGGLEALLDESKTOPS:
      ToggleAllDesktops();
      break;

   case VDM_MOVEWINDOW:
      SelectDesktopForWindow(this);
      break;

   case VDM_RESTORE:
      Restore();
      break;

   case VDM_MINIMIZE:
      Minimize();
      break;

   case VDM_MAXIMIZE:
      Maximize();
      break;

   case VDM_MAXIMIZEHEIGHT:
      MaximizeHeight();
      break;

   case VDM_MAXIMIZEWIDTH:
      MaximizeWidth();
      break;

   case VDM_CLOSE:
      PostMessage(m_hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
      break;

   case VDM_KILL:
      Kill();
      break;

   case VDM_PROPERTIES:
      DisplayWindowProperties();
      break;

   default:
      if (cmdId >= VDM_MOVETODESK && cmdId < VDM_MOVETODESK+deskMan->GetNbDesktops())
         MoveToDesktop(deskMan->GetDesktop(cmdId-VDM_MOVETODESK));
      break;
   }
}

void Window::SetMinimizeToTray(bool totray)
{
   m_MinToTray = totray;

   if (IsIconic() && IsOnCurrentDesk())
   {
      if (m_MinToTray)
      {
         // Move minimized icon from taskbar to tray
         trayManager->AddIcon(this);
         HideWindow();
      }
      else
      {
         // Move minimized icon from tray to taskbar
         trayManager->DelIcon(this);
         ShowWindow();
      }
   }
}

void Window::ToggleMinimizeToTray()
{
   SetMinimizeToTray(!IsMinimizeToTray());
}

void Window::ToggleOnTop()
{
   SetAlwaysOnTop(!IsAlwaysOnTop());
}

void Window::SetOnAllDesktops(bool onall)
{
   if (onall)
      MoveToDesktop(NULL);
   else if (IsOnAllDesktops())
      MoveToDesktop(deskMan->GetCurrentDesktop());
}

void Window::ToggleAllDesktops()
{
   SetOnAllDesktops(!IsOnAllDesktops());
}

void Window::SetTransparent(bool transp)
{
   m_transp.SetTransparencyLevel(transp ? m_transpLevel : (unsigned char)TRANSPARENCY_DISABLED);
}

void Window::ToggleTransparent()
{
   //TODO: will need to be updated once all settings have been changed
   if (GetTransparencyLevel() == TRANSPARENCY_DISABLED && !IsTransparent())
      SetTransparencyLevel(0xc0/*Settings::GetDefaultSetting(Settings::Window::TransparencyLevel)*/);
   SetTransparent(!IsTransparent());
}

void Window::SetTransparencyLevel(unsigned char level)
{
   //Update the variable
   m_transpLevel = level;

   //Refresh the display
   SetTransparent(IsTransparent());
}

void Window::Activate()
{
   if (IsIconic())
      Restore();

   if (!IsOnCurrentDesk())
      deskMan->SwitchToDesktop(m_desk);
   SetForegroundWindow(m_hOwnedWnd);
}

void Window::Restore()
{
   if (IsIconic())
   {
      m_iconic = false;

      if (IsOnCurrentDesk())
      {
         if (IsMinimizeToTray())
         {
            trayManager->DelIcon(this);
            ShowWindow();
         }
         else
            OpenIcon(m_hWnd);
      }
   }
   else if (IsZoomed(m_hWnd))
      ::ShowWindow(m_hWnd, SW_RESTORE);
}

void Window::Minimize()
{
   if (IsOnCurrentDesk())
   {
      if (IsMinimizeToTray())
      {
         trayManager->AddIcon(this);
         HideWindow();
      }
      else
         ::ShowWindow(m_hWnd, SW_MINIMIZE);
   }
   m_iconic = true;
}

void Window::Maximize()
{
   ::ShowWindow(m_hWnd, SW_MAXIMIZE);
}

void Window::MaximizeHeight()
{
   RECT rect;
   RECT screen;
   HWND hWnd = GetOwnedWindow();

   GetWindowRect(hWnd, &rect);
   SystemParametersInfo(SPI_GETWORKAREA, 0, &screen, 0);
   MoveWindow( hWnd, rect.left, screen.top,
               rect.right-rect.left, screen.bottom-screen.top,
               TRUE);
}

void Window::MaximizeWidth()
{
   RECT rect;
   RECT screen;
   HWND hWnd = GetOwnedWindow();

   GetWindowRect(hWnd, &rect);
   SystemParametersInfo(SPI_GETWORKAREA, 0, &screen, 0);
   MoveWindow( hWnd, screen.left, rect.top,
               screen.right-screen.left, rect.bottom - rect.top,
               TRUE);
}

void Window::Kill()
{
   HANDLE hProcess;
   DWORD pId;

   GetWindowThreadProcessId(m_hWnd, &pId);
   hProcess = OpenProcess( PROCESS_TERMINATE, 0, pId );
   if (hProcess == NULL)
      return;

   if (winMan->ConfirmKillWindow())
      TerminateProcess( hProcess, 9);
   CloseHandle (hProcess);
}

LRESULT Window::OnTrayIconMessage(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   switch(lParam)
   {
   case WM_RBUTTONDOWN:
   case WM_CONTEXTMENU:
      OnContextMenu();
      break;

   case WM_LBUTTONDOWN:
      OnMenuItemSelected(NULL, VDM_ACTIVATEWINDOW);
      break;
   }

   return 0;
}

void Window::OnContextMenu()
{
   HMENU hMenu;
   HRESULT res;
   POINT pt;

   hMenu = BuildMenu();

   GetCursorPos(&pt);
   SetForegroundWindow(vdWindow);
   res = TrackPopupMenu(hMenu, TPM_RETURNCMD|TPM_RIGHTBUTTON, pt.x, pt.y, 0, vdWindow, NULL);

   if (res >= WM_USER)
      OnMenuItemSelected(hMenu, res);
   else
      PostMessage(vdWindow, WM_COMMAND, res, 0);

   DestroyMenu(hMenu);
}

void Window::Hook()
{
   if (m_HookDllHandle)
      return;

   GetWindowThreadProcessId(m_hWnd, &m_dwProcessId);
   m_HookDllHandle = HookWindow(m_hWnd, m_dwProcessId, (int)this);
   if (m_HookDllHandle && m_hWnd != m_hOwnedWnd)
      HookWindow(m_hOwnedWnd, m_dwProcessId, (int)this);
}

void Window::UnHook()
{
   if (m_HookDllHandle)
   {
      UnHookWindow(m_HookDllHandle, m_dwProcessId, m_hWnd);
      if (m_hWnd != m_hOwnedWnd)
         UnHookWindow(m_HookDllHandle, m_dwProcessId, m_hOwnedWnd);
   }
   m_HookDllHandle = NULL;
}

void Window::FlashWindow(void)
{
   if (!IsOnCurrentDesk() && !IsWindowFlashing())
   {
      m_BallonMsg = msgManager.Add("This window requires attention!\r\nClick here to activate it.",
                                   GetText(), (int)GetIcon(), &OnFlashBallonClick, (int)m_hWnd);
   }
}

void Window::UnFlashWindow(void)
{
   if (IsWindowFlashing())
   {
      msgManager.Remove(m_BallonMsg);
      m_BallonMsg = NULL;
   }
}

void Window::OnFlashBallonClick(BalloonNotification::Message /*msg*/, int data)
{
   Window * wnd = winMan->GetWindow((HWND)data);
   if (wnd)
   {
      wnd->UnFlashWindow();
      wnd->Activate();
   }
}
