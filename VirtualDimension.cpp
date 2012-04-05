/*
 * Virtual Dimension -  a free, fast, and feature-full virtual desktop manager
 * for the Microsoft Windows platform.
 * Copyright (C) 2003-2005 Francois Ferrand
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

// Virtual Dimension.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "VirtualDimension.h"
#include "settings.h"
#include "desktopmanager.h"
#include "WindowsManager.h"
#include <Windowsx.h>
#include "hotkeymanager.h"
#include "shellhook.h"
#include "tooltip.h"
#include <objbase.h>
#include "fastwindow.h"
#include "HotKeyControl.h"
#include "LinkControl.h"
#include "ExplorerWrapper.h"
#include <shellapi.h>
#include <assert.h>
#include "HookDLL.h"
#include "Locale.h"
#include "CmdLine.h"

// Global Variables:
HWND configBox = NULL;
Transparency * transp;
TrayIcon * trayIcon;
AlwaysOnTop * ontop;
ToolTip * tooltip;

VirtualDimension vdWindow;

// Forward function definition
HWND CreateConfigBox();

int APIENTRY _tWinMain( HINSTANCE hInstance,
                        HINSTANCE /*hPrevInstance*/,
                        LPTSTR    lpCmdLine,
                        int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;
	BOOL firstrun;

   InitCommonControls();
   CoInitialize ( NULL );

   firstrun = vdWindow.Start(hInstance, nCmdShow);
	if (!firstrun)
	{
      CommandLineParser parser;
   	parser.ParseCommandLine(lpCmdLine);
      return -1;
	}

   // Load accelerators
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_VIRTUALDIMENSION);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (IsWindow(configBox) && IsDialogMessage(configBox, &msg))
      {
         if (NULL == PropSheet_GetCurrentPageHwnd(configBox))
         {
            DestroyWindow(configBox);
            configBox = NULL;
         }
      }
      else if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

VirtualDimension::VirtualDimension()
{
   LoadString(m_hInstance, IDS_APP_TITLE, m_szTitle, MAX_LOADSTRING);
	LoadString(m_hInstance, IDC_VIRTUALDIMENSION, m_szWindowClass, MAX_LOADSTRING);
}

bool VirtualDimension::Start(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   RECT pos;
   Settings settings;
   HWND hwndPrev;
   DWORD dwStyle;

   // If a previous instance is running, activate
   // that instance and terminate this one.
   hwndPrev = FindWindow();
   if (hwndPrev != NULL)
   {
        SetForegroundWindow (hwndPrev);
        return false;
   }

   m_hInstance = hInstance;

   InitHotkeyControl();
   InitHyperLinkControl();

   // Register the window class
   RegisterClass();

   // Bind the message handlers
   SetCommandHandler(IDM_ABOUT, this, &VirtualDimension::OnCmdAbout);
   SetSysCommandHandler(IDM_ABOUT, this, &VirtualDimension::OnCmdAbout);
   SetCommandHandler(IDM_CONFIGURE, this, &VirtualDimension::OnCmdConfigure);
   SetSysCommandHandler(IDM_CONFIGURE, this, &VirtualDimension::OnCmdConfigure);
   SetCommandHandler(IDM_EXIT, this, &VirtualDimension::OnCmdExit);
   SetSysCommandHandler(SC_CLOSE, this, &VirtualDimension::OnCmdExit);
   SetCommandHandler(IDM_LOCKPREVIEWWND, this, &VirtualDimension::OnCmdLockPreviewWindow);
   SetSysCommandHandler(IDM_LOCKPREVIEWWND, this, &VirtualDimension::OnCmdLockPreviewWindow);
   SetCommandHandler(IDM_SHOWCAPTION, this, &VirtualDimension::OnCmdShowCaption);
   SetSysCommandHandler(IDM_SHOWCAPTION, this, &VirtualDimension::OnCmdShowCaption);

   SetMessageHandler(WM_DESTROY, this, &VirtualDimension::OnDestroy);
	SetMessageHandler(WM_ENDSESSION, this, &VirtualDimension::OnEndSession);
   SetMessageHandler(WM_MOVE, this, &VirtualDimension::OnMove);
   SetMessageHandler(WM_WINDOWPOSCHANGING, this, &VirtualDimension::OnWindowPosChanging);
	SetMessageHandler(WM_DISPLAYCHANGE, this, &VirtualDimension::OnDisplayChange);
	SetMessageHandler(WM_SHOWWINDOW, this, &VirtualDimension::OnShowWindow);

   SetMessageHandler(WM_LBUTTONDOWN, this, &VirtualDimension::OnLeftButtonDown);
   SetMessageHandler(WM_LBUTTONUP, this, &VirtualDimension::OnLeftButtonUp);
   SetMessageHandler(WM_LBUTTONDBLCLK, this, &VirtualDimension::OnLeftButtonDblClk);
   SetMessageHandler(WM_RBUTTONDOWN, this, &VirtualDimension::OnRightButtonDown);

   SetMessageHandler(WM_MEASUREITEM, this, &VirtualDimension::OnMeasureItem);
   SetMessageHandler(WM_DRAWITEM, this, &VirtualDimension::OnDrawItem);

   m_autoHideTimerId = CreateTimer(this, &VirtualDimension::OnTimer);
	SetMessageHandler(WM_ACTIVATEAPP, this, &VirtualDimension::OnActivateApp);

	SetMessageHandler(WM_MOUSEHOVER, this, &VirtualDimension::OnMouseHover);
	SetMessageHandler(WM_MOUSELEAVE, this, &VirtualDimension::OnMouseLeave);
	SetMessageHandler(WM_NCHITTEST, this, &VirtualDimension::OnNCHitTest);

   SetMessageHandler(WM_VD_HOOK_MENU_COMMAND, this, &VirtualDimension::OnHookMenuCommand);
   SetMessageHandler(WM_VD_PREPARE_HOOK_MENU, this, &VirtualDimension::OnPrepareHookMenu);
   SetMessageHandler(WM_VD_CHECK_MIN_TO_TRAY, this, &VirtualDimension::OnCheckMinToTray);

	// compare the window's style
   m_hasCaption = settings.LoadSetting(Settings::HasCaption);
   dwStyle = WS_POPUP | WS_SYSMENU | (m_hasCaption ? WS_CAPTION : WS_DLGFRAME);

	// Reload the window's position
   settings.LoadSetting(Settings::WindowPosition, &pos);
   AdjustWindowRectEx(&pos, dwStyle, FALSE, WS_EX_TOOLWINDOW);

	// Dock the window to the screen borders
	m_dockedBorders = settings.LoadSetting(Settings::DockedBorders);
	DockWindow(pos);

	// Create the main window
	Create( WS_EX_TOOLWINDOW, m_szWindowClass, m_szTitle, dwStyle,
           pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top,
           NULL, NULL, hInstance);
   if (!IsValid())
      return false;

   hWnd = *this;

	// Load some settings
	m_snapSize = settings.LoadSetting(Settings::SnapSize);
	m_autoHideDelay = settings.LoadSetting(Settings::AutoHideDelay);
	m_shrinked = false;

	m_tracking = false;

	//Ensure the window gets docked if it is close enough to the borders
	SetWindowPos(hWnd, NULL, pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOOWNERZORDER);

   // Setup the system menu
   m_pSysMenu = GetSystemMenu(hWnd, FALSE);
	if (m_pSysMenu != NULL)
	{
      RemoveMenu(m_pSysMenu, SC_RESTORE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, SC_MINIMIZE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, SC_MAXIMIZE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, SC_MOVE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, SC_SIZE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, 0, MF_BYCOMMAND);

      AppendMenu(m_pSysMenu, MF_SEPARATOR, 0, NULL);
      AppendMenu(m_pSysMenu, MF_STRING, IDM_CONFIGURE, Locale::GetInstance().GetString(IDS_CONFIGURE)); //"C&onfigure"
      AppendMenu(m_pSysMenu, MF_STRING, IDM_LOCKPREVIEWWND, Locale::GetInstance().GetString(IDS_LOCKPREVIEWWND)); //"&Lock the window"
      AppendMenu(m_pSysMenu, MF_STRING, IDM_SHOWCAPTION, Locale::GetInstance().GetString(IDS_SHOWCAPTION)); //"S&how the caption"

      if (CreateLangMenu())
         AppendMenu(m_pSysMenu, MF_STRING|MF_POPUP, (UINT_PTR)m_pLangMenu, Locale::GetInstance().GetString(IDS_LANGUAGEMENU)); //"L&anguage"
      AppendMenu(m_pSysMenu, MF_STRING, IDM_ABOUT, Locale::GetInstance().GetString(IDS_ABOUT)); //"&About"
      CheckMenuItem(m_pSysMenu, IDM_SHOWCAPTION, m_hasCaption ? MF_CHECKED : MF_UNCHECKED );
   }

   // Lock the preview window as appropriate
   LockPreviewWindow(settings.LoadSetting(Settings::LockPreviewWindow));

   // Bind to explorer
   explorerWrapper = new ExplorerWrapper(this);

   // Initialize the tray icon manager
   trayManager = new TrayIconsManager();

   // Initialize tray icon
   trayIcon = new TrayIcon(hWnd);

   // Initialize transparency (set value two times, to make a fade-in)
   transp = new Transparency(hWnd);
	transp->SetTransparencyLevel(0);
   transp->SetTransparencyLevel(settings.LoadSetting(Settings::TransparencyLevel), true);

   // Initialize always on top state
   ontop = new AlwaysOnTop(hWnd);
   ontop->SetAlwaysOnTop(settings.LoadSetting(Settings::AlwaysOnTop));

   // Create the tooltip
   tooltip = new ToolTip(hWnd);

   // Create mouse warp
   mousewarp = new MouseWarp();

   // Create the windows manager
   winMan = new WindowsManager;

   // Create the desk manager
	settings.LoadSetting(Settings::WindowPosition, &pos);	//use client position
   deskMan = new DesktopManager(pos.right - pos.left, pos.bottom - pos.top);

   // Retrieve the initial list of windows
   winMan->PopulateInitialWindowsSet();

   //Update tray icon tooltip
   trayIcon->Update();

	//Bind some additional message handlers (which need the desktop manager)
   SetMessageHandler(WM_SIZE, this, &VirtualDimension::OnSize);
   SetMessageHandler(WM_PAINT, deskMan, &DesktopManager::OnPaint);

   // Show window if needed
   if (m_isWndVisible = (settings.LoadSetting(Settings::ShowWindow) || !trayIcon->HasIcon()))
   {
      ShowWindow(hWnd, nCmdShow);
      Refresh();
   }

   return true;
}

VirtualDimension::~VirtualDimension()
{
}

void VirtualDimension::LockPreviewWindow(bool lock)
{
   LONG_PTR style;

   m_lockPreviewWindow = lock;

   CheckMenuItem(m_pSysMenu, IDM_LOCKPREVIEWWND, m_lockPreviewWindow ? MF_CHECKED : MF_UNCHECKED );

   style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
   if (m_lockPreviewWindow)
   {
      style &= ~WS_THICKFRAME;

      RemoveMenu(m_pSysMenu, SC_MOVE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, SC_SIZE, MF_BYCOMMAND);
   }
   else
   {
      style |= WS_THICKFRAME;

      InsertMenu(m_pSysMenu, 0, MF_BYPOSITION, SC_SIZE, Locale::GetInstance().GetString(IDS_MENU_SIZE)); // "&Size"
      InsertMenu(m_pSysMenu, 0, MF_BYPOSITION, SC_MOVE, Locale::GetInstance().GetString(IDS_MENU_MOVE)); // "&Move"
   }
   SetWindowLongPtr(m_hWnd, GWL_STYLE, style);
}

void VirtualDimension::ShowCaption(bool caption)
{
   LONG_PTR style;

   m_hasCaption = caption;

   CheckMenuItem(m_pSysMenu, IDM_SHOWCAPTION, m_hasCaption ? MF_CHECKED : MF_UNCHECKED );

   style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
   if (m_hasCaption)
   {
      style &= ~WS_DLGFRAME;
      style |= WS_CAPTION;
   }
   else
   {
      style &= ~WS_CAPTION;
      style |= WS_DLGFRAME;
   }
   SetWindowLongPtr(m_hWnd, GWL_STYLE, style);
   SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
}

ATOM VirtualDimension::RegisterClass()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_SAVEBITS;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInstance;
	wcex.hIcon			= LoadIcon(m_hInstance, (LPCTSTR)IDI_VIRTUALDIMENSION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground	= (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= m_szWindowClass;
   wcex.hIconSm      = NULL;

   return FastWindow::RegisterClassEx(&wcex);
}

LRESULT VirtualDimension::OnCmdAbout(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   DialogBox(vdWindow, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);

   return 0;
}

LRESULT VirtualDimension::OnCmdLockPreviewWindow(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   LockPreviewWindow(!IsPreviewWindowLocked());

   return 0;
}

LRESULT VirtualDimension::OnCmdShowCaption(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   ShowCaption(!HasCaption());

   return 0;
}

LRESULT VirtualDimension::OnCmdConfigure(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   if (!configBox)
      configBox = CreateConfigBox();

   return 0;
}

LRESULT VirtualDimension::OnCmdExit(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	//todo: fade-out
	DestroyWindow(hWnd);
   return 0;
}

LRESULT VirtualDimension::OnLeftButtonDown(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   POINT pt;
   BOOL screenPos = FALSE;

   pt.x = GET_X_LPARAM(lParam);
   pt.y = GET_Y_LPARAM(lParam);

	if (m_shrinked)
	{
		if (!IsPreviewWindowLocked() &&         //for performance reasons only
			 (ClientToScreen(hWnd, &pt)) &&
			 (DragDetect(hWnd, pt)))
		{
			//trick windows into thinking we are dragging the title bar, to let the user move the window
			m_draggedWindow = NULL;
			m_dragCursor = NULL;
			ReleaseCapture();
			::SendMessage(hWnd,WM_NCLBUTTONDOWN,HTCAPTION,(LPARAM)&pt);
		}
		else
			UnShrink();
	}
	else
	{
		//Stop the hide timer, to ensure the window does not get hidden
      KillTimer(m_autoHideTimerId);

		//Find the item under the mouse, and check if it's being dragged
		Desktop * desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
		if ( (desk) &&
			((m_draggedWindow = desk->GetWindowFromPoint(pt.x, pt.y)) != NULL) &&
			(!m_draggedWindow->IsOnDesk(NULL)) &&
			((screenPos = ClientToScreen(hWnd, &pt)) != FALSE) &&
			(DragDetect(hWnd, pt)) )
		{
			ICONINFO icon;

			//Dragging a window's icon
			SetCapture(hWnd);

			GetIconInfo(m_draggedWindow->GetIcon(), &icon);
			icon.fIcon = FALSE;
			m_dragCursor = (HCURSOR)CreateIconIndirect(&icon);
			SetCursor(m_dragCursor);
		}
		else if (!IsPreviewWindowLocked() &&         //for performance reasons only
					(screenPos || ClientToScreen(hWnd, &pt)) &&
					(DragDetect(hWnd, pt)))
		{
			//trick windows into thinking we are dragging the title bar, to let the user move the window
			m_draggedWindow = NULL;
			m_dragCursor = NULL;
			ReleaseCapture();
			::SendMessage(hWnd,WM_NCLBUTTONDOWN,HTCAPTION,(LPARAM)&pt);
		}
		else
		{
			//switch to the desktop that was clicked
			m_draggedWindow = NULL;
			deskMan->SwitchToDesktop(desk);
		}
	}

   return 0;
}

LRESULT VirtualDimension::OnLeftButtonUp(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   POINT pt;

   //If not dragging a window, nothing to do
   if (m_draggedWindow == NULL)
      return 0;

   //Release capture
   ReleaseCapture();

   //Free the cursor
   DestroyCursor(m_dragCursor);

   pt.x = GET_X_LPARAM(lParam);
   pt.y = GET_Y_LPARAM(lParam);

   //Find out the target desktop
   Desktop * desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
   if (m_draggedWindow->IsOnDesk(desk))
      return 0;   //window already on the target desk

   //Move the window to this desktop
   m_draggedWindow->MoveToDesktop(desk);

   //Refresh the window
   Refresh();

   return 0;
}

LRESULT VirtualDimension::OnLeftButtonDblClk(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   POINT pt;
   Window * window;
   Desktop * desk;

	if (m_shrinked)
		return 0;

   pt.x = GET_X_LPARAM(lParam);
   pt.y = GET_Y_LPARAM(lParam);

   desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
   if ( (desk) &&
        ((window = desk->GetWindowFromPoint(pt.x, pt.y)) != NULL) )
      window->Activate();
   return 0;
}

LRESULT VirtualDimension::OnRightButtonDown(HWND hWnd, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
   HMENU hMenu = NULL, hBaseMenu;
   POINT pt;
   HRESULT res;

   pt.x = GET_X_LPARAM(lParam);
   pt.y = GET_Y_LPARAM(lParam);

	//Stop the hide timer, to ensure the window does not get hidden
	KillTimer(m_autoHideTimerId);

   //Get the context menu
   Desktop * desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
   Window * window = NULL;
   if ((!m_shrinked) &&
		 ((wParam & MK_CONTROL) == 0) &&
       (desk != NULL))
   {
      window = desk->GetWindowFromPoint(pt.x, pt.y);
      if (window)
         hMenu = window->BuildMenu();
      else
         hMenu = desk->BuildMenu();
   }

   //If no window on desktop, or no menu for the window, display system menu
   if (hMenu == NULL || GetMenuItemCount(hMenu) == 0)
   {
      hBaseMenu = hMenu; //destroy the newly created menu, even if we don't use it
      hMenu = m_pSysMenu;
   }
   else
      hBaseMenu = hMenu;

   assert(hMenu != NULL);

   //And show the menu
   ClientToScreen(hWnd, &pt);
   res = TrackPopupMenu(hMenu, TPM_RETURNCMD|TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

   //Process the resulting message
   if (hMenu == m_pSysMenu)
      PostMessage(hWnd, WM_SYSCOMMAND, res, 0);
   else if (res >= WM_USER)
   {
      if (window != NULL)
         window->OnMenuItemSelected(hMenu, res);
      else
         desk->OnMenuItemSelected(hMenu, res);
   }
   else
      PostMessage(hWnd, WM_COMMAND, res, 0);

   DestroyMenu(hBaseMenu);

   return 0;
}

LRESULT VirtualDimension::OnDestroy(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   RECT pos;
   Settings settings;

   // Before exiting, save the window position
   pos.left = m_location.x;
   pos.top = m_location.y;
   pos.right = m_location.x + deskMan->GetWindowWidth();
   pos.bottom = m_location.y + deskMan->GetWindowHeight();
   settings.SaveSetting(Settings::WindowPosition, &pos);
	settings.SaveSetting(Settings::DockedBorders, m_dockedBorders);

	//Save the snap size
	settings.SaveSetting(Settings::SnapSize, m_snapSize);

	//Save the auto-hide delay
	settings.SaveSetting(Settings::AutoHideDelay, m_autoHideDelay);

	//Save the visibility state of the window before it is hidden
	settings.SaveSetting(Settings::ShowWindow, m_isWndVisible);

   //Save the locking state of the window
   settings.SaveSetting(Settings::LockPreviewWindow, IsPreviewWindowLocked());

   //Save the visibility state of the title bar
   settings.SaveSetting(Settings::HasCaption, HasCaption());

   // Remove the tray icon
   delete trayIcon;

   // Cleanup transparency
   settings.SaveSetting(Settings::TransparencyLevel, transp->GetTransparencyLevel());
   delete transp;

   // Cleanup always on top state
   settings.SaveSetting(Settings::AlwaysOnTop, ontop->IsAlwaysOnTop());
   delete ontop;

   // Destroy the tooltip
   delete tooltip;

   // Destroy the mouse warp
   delete mousewarp;

   // Destroy the desktop manager
   delete deskMan;

   // Destroy the windows manager
   delete winMan;

   // Destroy the tray icons manager
   delete trayManager;

   PostQuitMessage(0);

   return 0;
}

LRESULT VirtualDimension::OnMeasureItem(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;

   if (wParam != 0)
      return DefWindowProc(hWnd, message, wParam, lParam);

   lpmis->itemHeight = 16;
   lpmis->itemWidth = 16;

   return TRUE;
}

LRESULT VirtualDimension::OnDrawItem(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

   if (wParam != 0)
      return DefWindowProc(hWnd, message, wParam, lParam);

   DrawIconEx(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, (HICON)lpdis->itemData, 16, 16, 0, NULL, DI_NORMAL);

   return TRUE;
}

LRESULT VirtualDimension::OnHookMenuCommand(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
   Window * win = (Window*)lParam;

   win->OnMenuItemSelected(NULL, (int)wParam);
   if (win->IsOnCurrentDesk())
      SetForegroundWindow(win->GetOwnedWindow());

   return TRUE;
}

LRESULT VirtualDimension::OnPrepareHookMenu(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
   return ((Window*)lParam)->PrepareSysMenu((HANDLE)wParam);
}

LRESULT VirtualDimension::OnCheckMinToTray(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   return ((Window*)lParam)->IsMinimizeToTray();
}

LRESULT VirtualDimension::OnEndSession(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM /*lParam*/)
{
   if (wParam)
      //The session is ending -> destroy the window
      DestroyWindow(m_hWnd);	//fade-out ?

   return 0;
}

LRESULT VirtualDimension::OnMove(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
	if (!m_shrinked)
	{
		m_location.x = (int)(short) LOWORD(lParam);
		m_location.y = (int)(short) HIWORD(lParam);
	}

   return 0;
}

LRESULT VirtualDimension::OnWindowPosChanging(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   RECT	deskRect;
   WINDOWPOS * lpwndpos = (WINDOWPOS*)lParam;

	// No action if the window is not moved or sized
	if ((lpwndpos->flags & SWP_NOMOVE) && (lpwndpos->flags & SWP_NOSIZE))
		return TRUE;

	// Get work area dimensions
	SystemParametersInfo(SPI_GETWORKAREA, 0, &deskRect, 0);

	if (!m_shrinked)
	{
		// Snap to screen border
		m_dockedBorders = 0;
		if( (lpwndpos->x >= -m_snapSize + deskRect.left) &&
			(lpwndpos->x <= deskRect.left + m_snapSize) )
		{
			//Left border
			lpwndpos->x = deskRect.left;
			m_dockedBorders |= DOCK_LEFT;
		}
		if( (lpwndpos->y >= -m_snapSize + deskRect.top) &&
			(lpwndpos->y <= deskRect.top + m_snapSize) )
		{
			// Top border
			lpwndpos->y = deskRect.top;
			m_dockedBorders |= DOCK_TOP;
		}
		if( (lpwndpos->x + lpwndpos->cx <= deskRect.right + m_snapSize) &&
			(lpwndpos->x + lpwndpos->cx >= deskRect.right - m_snapSize) )
		{
			// Right border
			lpwndpos->x = deskRect.right - lpwndpos->cx;
			m_dockedBorders |= DOCK_RIGHT;
		}
		if( (lpwndpos->y + lpwndpos->cy <= deskRect.bottom + m_snapSize) &&
			(lpwndpos->y + lpwndpos->cy >= deskRect.bottom - m_snapSize) )
		{
			// Bottom border
			lpwndpos->y = deskRect.bottom - lpwndpos->cy;
			m_dockedBorders |= DOCK_BOTTOM;
		}
	}
	else
	{
		//Constrain to borders
		if (lpwndpos->x < deskRect.left)
			lpwndpos->x = deskRect.left;
		if (lpwndpos->x+lpwndpos->cx > deskRect.right)
			lpwndpos->x = deskRect.right - lpwndpos->cx;
		if (lpwndpos->y < deskRect.top)
			lpwndpos->y = deskRect.top;
		if (lpwndpos->y+lpwndpos->cy > deskRect.bottom)
			lpwndpos->y = deskRect.bottom - lpwndpos->cy;

		int xdist = min(lpwndpos->x-deskRect.left, deskRect.right-lpwndpos->x-lpwndpos->cx) >> 4;
		int ydist = min(lpwndpos->y-deskRect.top, deskRect.bottom-lpwndpos->y-lpwndpos->cy) >> 4;

		m_dockedBorders = 0;
		if (xdist <= ydist)
		{
			//Dock to left/right
			if (2*lpwndpos->x+lpwndpos->cx > deskRect.right-deskRect.left)
			{
				//dock to right
				lpwndpos->x = deskRect.right - lpwndpos->cx;
				m_dockedBorders |= DOCK_RIGHT;
			}
			else
			{
				//dock to left
				lpwndpos->x = deskRect.left;
				m_dockedBorders |= DOCK_LEFT;
			}
		}
		if (xdist >= ydist)
		{
			//Dock to top/bottom
			if (2*lpwndpos->y+lpwndpos->cy > deskRect.bottom-deskRect.top)
			{
				//dock to bottom
				lpwndpos->y = deskRect.bottom - lpwndpos->cy;
				m_dockedBorders |= DOCK_BOTTOM;
			}
			else
			{
				//dock to top
				lpwndpos->y = deskRect.top;
				m_dockedBorders |= DOCK_TOP;
			}
		}
	}

   return TRUE;
}

/** Update the rectangle, to dock the window.
 * @return true if the docking caused the rect to change, else false.
 */
bool VirtualDimension::DockWindow(RECT & pos)
{
	RECT deskRect;
	bool res = false;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &deskRect, 0);
	if (m_dockedBorders & DOCK_LEFT)
	{
		pos.right -= pos.left - deskRect.left;
		pos.left = deskRect.left;
		res = true;
	}
	if (m_dockedBorders & DOCK_RIGHT)
	{
		if (!(m_dockedBorders & DOCK_LEFT))
			pos.left -= pos.right - deskRect.right;
		pos.right = deskRect.right;
		res = true;
	}
	if (m_dockedBorders & DOCK_TOP)
	{
		pos.bottom -= pos.top - deskRect.top;
		pos.top = deskRect.top;
		res = true;
	}
	if (m_dockedBorders & DOCK_BOTTOM)
	{
		if (!(m_dockedBorders & DOCK_TOP))
			pos.top -= pos.bottom - deskRect.bottom;
		pos.bottom = deskRect.bottom;
		res = true;
	}
	return res;
}

LRESULT VirtualDimension::OnDisplayChange(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	RECT  pos;
	GetWindowRect(m_hWnd, &pos);
	if (DockWindow(pos))
	{
		if (!m_shrinked && m_autoHideDelay > 0)
			SetTimer(m_autoHideTimerId, m_autoHideDelay);	//reset the timer, to avoid hiding the window during the resolution change, as it does not look very nice
		MoveWindow(m_hWnd, pos.left, pos.top, pos.right-pos.left, pos.bottom-pos.top, TRUE);
	}
	return 0;
}

LRESULT VirtualDimension::OnShowWindow(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM /*lParam*/)
{
	m_isWndVisible = (wParam != FALSE);
	return 0;
}

LRESULT VirtualDimension::OnTimer(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   POINT pt;

   //Do not shrink and let the timer run if the mouse is over the window -> it will be hidden later, when
   //mouse is not on window anymore.
   GetCursorPos(&pt);
   if (!IsPointInWindow(pt) && GetWindowThreadProcessId(GetForegroundWindow(),NULL) != GetCurrentThreadId())
   {
      KillTimer(m_autoHideTimerId); //already auto-hidden -> do not need to
	   Shrink();
   }

   return 0;
}

LRESULT VirtualDimension::OnActivateApp(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
	if (wParam == TRUE)
      KillTimer(m_autoHideTimerId);                   //Kill auto-hide timer if activated
	else if (m_autoHideDelay > 0 && ((DWORD)lParam != GetCurrentThreadId()))
		SetTimer(m_autoHideTimerId, m_autoHideDelay);   //Re-start auto-hide timer if de-activated
	return 0;
}

LRESULT VirtualDimension::OnPaint(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	//This method is used to paint the shrinked window
	PAINTSTRUCT ps;
	RECT rect;
	HDC hdc;

	GetClientRect(hWnd, &rect);
	hdc = BeginPaint(hWnd, &ps);

	Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT VirtualDimension::OnSize(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
   if ((!m_shrinked) && (wParam == SIZE_RESTORED))
		deskMan->ReSize(LOWORD(lParam), HIWORD(lParam));

	return 0;
}

LRESULT VirtualDimension::OnMouseHover(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	//Un-shrink the window
	if (m_shrinked)
		UnShrink();

	m_tracking = false;
	return 0;
}

LRESULT VirtualDimension::OnMouseLeave(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   //Set timer to auto-hide
	if (!m_shrinked && m_autoHideDelay > 0)
		SetTimer(m_autoHideTimerId, m_autoHideDelay);

	m_tracking = false;
	return 0;
}

LRESULT VirtualDimension::OnNCHitTest(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Stop auto-hide timer (re-entry in the window)
	KillTimer(m_autoHideTimerId);

	//Track mouse hover/leave, if not already doing so
	if (!m_tracking)
	{
		//Setup mouse tracking
		TRACKMOUSEEVENT tme;

		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 1000;
		tme.hwndTrack = m_hWnd;
		m_tracking = TrackMouseEvent(&tme) ? true : false;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT VirtualDimension::OnCmdLanguageChange(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM /*lParam*/)
{
    // the current language code + wm_cd_language should give the current checked menu item ...
    int iPreviousLanguageCode = Locale::GetInstance().GetLanguage();
    if (Locale::GetInstance().SetLanguage(wParam-WM_VD_LANGUAGE))
    {
       UpdateSystemMenu();
       if (iPreviousLanguageCode > 0)
           CheckMenuItem(m_pLangMenu,(UINT)iPreviousLanguageCode+WM_VD_LANGUAGE,MF_BYCOMMAND|MF_UNCHECKED);
       CheckMenuItem(m_pLangMenu,(UINT)wParam,MF_BYCOMMAND|MF_CHECKED);
    }
    return 0;
}

#define SHRUNK_THICKNESS 10

void VirtualDimension::Shrink(void)
{
	RECT pos, deskRect;
	DWORD style;

	if (m_shrinked || !m_dockedBorders)
		return;

	m_shrinked = true;

	//Compute the position where to display the handle
	SystemParametersInfo(SPI_GETWORKAREA, 0, &deskRect, 0);
	GetWindowRect(m_hWnd, &pos);

	switch(m_dockedBorders & (DOCK_LEFT|DOCK_RIGHT))
	{
	case DOCK_LEFT:
		pos.right = deskRect.left + SHRUNK_THICKNESS;
		pos.left = pos.right - SHRUNK_THICKNESS;
		break;

	case DOCK_RIGHT:
		pos.left = deskRect.right - SHRUNK_THICKNESS;
		pos.right = pos.left + SHRUNK_THICKNESS;
		break;

	case DOCK_LEFT|DOCK_RIGHT:
		pos.left = deskRect.left;
		pos.right = deskRect.right;
		break;

	default:
		break;
	}

	switch(m_dockedBorders & (DOCK_TOP|DOCK_BOTTOM))
	{
	case DOCK_TOP:
		pos.bottom = deskRect.top + SHRUNK_THICKNESS;
		pos.top = pos.bottom - SHRUNK_THICKNESS;
		break;

	case DOCK_BOTTOM:
		pos.top = deskRect.bottom - SHRUNK_THICKNESS;
		pos.bottom = pos.top + SHRUNK_THICKNESS;
		break;

	case DOCK_TOP|DOCK_BOTTOM:
		pos.top = deskRect.top;
		pos.bottom = deskRect.bottom;
		break;

	default:
		break;
	}

	//Change the method to use for painting the window
	SetMessageHandler(WM_PAINT, this, &VirtualDimension::OnPaint);

	//Change the style of the window
	style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
	style &= ~WS_CAPTION;
	style &= ~WS_DLGFRAME;
	style &= ~WS_BORDER;
	style &= ~WS_THICKFRAME;
	SetWindowLongPtr(m_hWnd, GWL_STYLE, style);

   RemoveMenu(m_pSysMenu, SC_MOVE, MF_BYCOMMAND);
   RemoveMenu(m_pSysMenu, SC_SIZE, MF_BYCOMMAND);

	//Apply the changes
	SetWindowPos(m_hWnd, NULL, pos.left, pos.top, pos.right-pos.left, pos.bottom-pos.top, SWP_NOZORDER | SWP_FRAMECHANGED);

	//Disable tooltips
	tooltip->ShowTooltips(false);

	//Refresh the display
	Refresh();
}

void VirtualDimension::UnShrink(void)
{
	RECT pos;
	DWORD style;

	if (!m_shrinked)
	return;

	//Change the method to use for painting the window
	SetMessageHandler(WM_PAINT, deskMan, &DesktopManager::OnPaint);

	//Restore the window's style
	style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
	style |= (m_hasCaption ? WS_CAPTION : WS_DLGFRAME);
	style |= (m_lockPreviewWindow ? 0 : WS_THICKFRAME);
	SetWindowLongPtr(m_hWnd, GWL_STYLE, style);

	if (!m_lockPreviewWindow)
	{
		InsertMenu(m_pSysMenu, 0, MF_BYPOSITION, SC_SIZE, "&Size");
		InsertMenu(m_pSysMenu, 0, MF_BYPOSITION, SC_MOVE, "&Move");
	}

	//Restore the windows position
	pos.left = m_location.x;
	pos.right = pos.left + deskMan->GetWindowWidth();
	pos.top = m_location.y;
	pos.bottom = pos.top + deskMan->GetWindowHeight();
	AdjustWindowRectEx(&pos, GetWindowLongPtr(m_hWnd, GWL_STYLE), FALSE, GetWindowLongPtr(m_hWnd, GWL_EXSTYLE));

	//Apply the changes
	SetWindowPos(m_hWnd, NULL, pos.left, pos.top, pos.right-pos.left, pos.bottom-pos.top, SWP_DRAWFRAME | SWP_NOZORDER | SWP_FRAMECHANGED);

	//Enable tooltips
	tooltip->ShowTooltips(true);

	//Refresh the display
	Refresh();

	m_shrinked = false;
}

bool VirtualDimension::IsPointInWindow(POINT pt)
{
   RECT rect;
   GetWindowRect(vdWindow, &rect);
   return PtInRect(&rect, pt) ? true : false;
}

bool VirtualDimension::CreateLangMenu()
{
	LocalesIterator it;
	int count = 0;
	// languageCode from registry
	int currentLanguageCode = Locale::GetInstance().GetLanguage();

	//Create the menu
	m_pLangMenu = CreatePopupMenu();

	//Add the entries
	while(m_pLangMenu && it.GetNext())
	{
		String name;
		HICON hicon;
		name = it.GetLanguage(&hicon, NULL);
		if (!name.empty())
		{
			MENUITEMINFO iteminfo;
			int code = it.GetLanguageCode();

			count++;

			iteminfo.cbSize = sizeof(MENUITEMINFO);
			iteminfo.fMask = MIIM_DATA|MIIM_STRING|MIIM_FTYPE|MIIM_BITMAP|MIIM_ID;
			iteminfo.dwTypeData = (LPSTR)name.c_str();
			iteminfo.fType = MFT_STRING;
			iteminfo.dwItemData = (ULONG_PTR)hicon;
			iteminfo.hbmpItem = HBMMENU_CALLBACK;
			iteminfo.wID = WM_VD_LANGUAGE+code; // in order to get WM_COMMAND msg
			InsertMenuItem(m_pLangMenu, WM_VD_LANGUAGE+code, FALSE, &iteminfo);
			if (code == currentLanguageCode)
			    CheckMenuItem(m_pLangMenu,(UINT)code+WM_VD_LANGUAGE,MF_BYCOMMAND|MF_CHECKED);

			// then we connect any menu to the window proc
         SetCommandHandler(WM_VD_LANGUAGE+code, this, &VirtualDimension::OnCmdLanguageChange);
         SetSysCommandHandler(WM_VD_LANGUAGE+code, this, &VirtualDimension::OnCmdLanguageChange);
		}
	}

	return count > 1;
}

void RenameMenu(HMENU hMenu, UINT cmdId, UINT textId)
{
   UINT state = GetMenuState(hMenu, cmdId, MF_BYCOMMAND);
   ModifyMenu(hMenu, cmdId, MF_BYCOMMAND|state, cmdId, Locale::GetInstance().GetString(textId));
}

void VirtualDimension::UpdateSystemMenu()
{
   if (m_pSysMenu)
   {
      RenameMenu(m_pSysMenu, SC_SIZE, IDS_MENU_SIZE); // "&Size"
      RenameMenu(m_pSysMenu, SC_MOVE, IDS_MENU_MOVE); // "&Move"
      RenameMenu(m_pSysMenu, IDM_CONFIGURE, IDS_CONFIGURE); //"C&onfigure"
      RenameMenu(m_pSysMenu, IDM_LOCKPREVIEWWND, IDS_LOCKPREVIEWWND); //"&Lock the window"
      RenameMenu(m_pSysMenu, IDM_SHOWCAPTION, IDS_SHOWCAPTION); //"S&how the caption"

      if (m_pLangMenu && GetMenuItemCount(m_pLangMenu)>1)
         RenameMenu(m_pSysMenu, (UINT_PTR)m_pLangMenu, IDS_LANGUAGEMENU); //"L&anguage"
      RenameMenu(m_pSysMenu, IDM_ABOUT, IDS_ABOUT); //"&About"
   }
}

// Message handler for about box.
LRESULT CALLBACK VirtualDimension::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   static IPicture * picture;

	switch (message)
	{
	case WM_INITDIALOG:
      SetFocus(GetDlgItem(hDlg, IDOK));
      picture = PlatformHelper::OpenImage(MAKEINTRESOURCE(IDI_VIRTUALDIMENSION));

      TCHAR text[MAX_PATH];
      DWORD dwHandle;
      DWORD vinfSize;
      LPVOID lpVersionInfo;
      TCHAR * lpVal;
      UINT dwValSize;

      GetModuleFileName(NULL, text, MAX_PATH);
      vinfSize = GetFileVersionInfoSize(text, &dwHandle);
      lpVersionInfo = malloc(vinfSize);
      GetFileVersionInfo(text, dwHandle, vinfSize, lpVersionInfo);

      VerQueryValue(lpVersionInfo, "\\StringFileInfo\\040904b0\\ProductName", (LPVOID*)&lpVal, &dwValSize);
      strncpy(text, lpVal, dwValSize);
      strcat(text, " v");
      VerQueryValue(lpVersionInfo, "\\StringFileInfo\\040904b0\\ProductVersion", (LPVOID*)&lpVal, &dwValSize);
      lpVal = strtok(lpVal, ", \t");
      strcat(text, lpVal);
      strcat(text, ".");
      lpVal = strtok(NULL, ", \t");
      strcat(text, lpVal);
		lpVal = strtok(NULL, ", \t");
		if (*lpVal != '0')
		{
			*lpVal += 'a' - '0';
			strcat(text, lpVal);
		}
      SetDlgItemText(hDlg, IDC_PRODUCT, text);

      VerQueryValue(lpVersionInfo, "\\StringFileInfo\\040904b0\\LegalCopyright", (LPVOID*)&lpVal, &dwValSize);
      strncpy(text, lpVal, dwValSize);
      SetDlgItemText(hDlg, IDC_COPYRIGHT, text);

      free(lpVersionInfo);
      return FALSE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
      case IDOK:
      case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
         if (picture)
         {
            picture->Release();
            picture = NULL;
         }
			return TRUE;

      case IDC_HOMEPAGE_LINK:
         if (HIWORD(wParam) == STN_CLICKED)
         {
            ShellExecute(hDlg, "open", "http://virt-dimension.sourceforge.net",
                         NULL, NULL, SW_SHOWNORMAL);
         }
         break;

      case IDC_GPL_LINK:
         if (HIWORD(wParam) == STN_CLICKED)
         {
            ShellExecute(hDlg, "open", "LICENSE.html",
                         NULL, NULL, SW_SHOWNORMAL);
         }
         break;
		}
		break;

   case WM_DRAWITEM:
      if (picture)
         PlatformHelper::CustomDrawIPicture(picture, (LPDRAWITEMSTRUCT)lParam, false);
      return TRUE;
	}
	return FALSE;
}
