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

#ifndef __VIRTUAL_DIMENSION_H__
#define __VIRTUAL_DIMENSION_H__

#include "resource.h"
#include "transparency.h"
#include "trayicon.h"
#include "alwaysontop.h"
#include "ToolTip.h"
#include "FastWindow.h"
#include "Window.h"
#include "MouseWarp.h"

extern HWND configBox;
extern Transparency * transp;
extern TrayIcon * trayIcon;
extern AlwaysOnTop * ontop;
extern ToolTip * tooltip;
extern MouseWarp * mousewarp;

class VirtualDimension: public FastWindow
{
public:
   VirtualDimension();
   ~VirtualDimension();

   bool Start(HINSTANCE hInstance, int nCmdShow);
   HWND GetHWND() const   { return *((FastWindow*)this); }
   HMENU GetMenu() const  { return m_pSysMenu; }
   operator HINSTANCE()   { return m_hInstance; }

   HWND FindWindow() const	  { return ::FindWindow(m_szWindowClass, m_szTitle); }

   inline void Refresh()  { InvalidateRect(m_hWnd, NULL, FALSE); }

	inline int GetSnapSize() const		{ return m_snapSize; }
	inline void SetSnapSize(int size)	{ m_snapSize = size; }

	int GetAutoHideDelay() const			{ return m_autoHideDelay; }
	void SetAutoHideDelay(int delay)		{ m_autoHideDelay = delay; }

	void Shrink(void);
	void UnShrink(void);

   bool IsPointInWindow(POINT pt);

protected:
   Window * m_draggedWindow;
   HCURSOR m_dragCursor;

   HMENU m_pSysMenu;
	HMENU m_pLangMenu;

   HINSTANCE m_hInstance;

   static const int MAX_LOADSTRING = 100;
   TCHAR m_szTitle[MAX_LOADSTRING];
   TCHAR m_szWindowClass[MAX_LOADSTRING];
   POINT m_location;

	enum DockPosition {
		DOCK_LEFT	= 1,
		DOCK_RIGHT	= 2,
		DOCK_TOP		= 4,
		DOCK_BOTTOM	= 8,
	};
	int m_dockedBorders;

	int m_snapSize;

	bool m_tracking;

	bool m_shrinked;
   bool m_lockPreviewWindow;
   bool m_hasCaption;
	bool m_isWndVisible;
	int m_autoHideDelay;
   UINT_PTR m_autoHideTimerId;

   bool IsPreviewWindowLocked() const     { return m_lockPreviewWindow; }
   void LockPreviewWindow(bool lock);

   bool HasCaption() const                { return m_hasCaption; }
   void ShowCaption(bool caption);

   ATOM RegisterClass();

	bool CreateLangMenu();
	void UpdateSystemMenu();

	bool DockWindow(RECT & rect);

   LRESULT OnCmdAbout(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnCmdLockPreviewWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnCmdShowCaption(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnCmdConfigure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnCmdExit(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   LRESULT OnLeftButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnLeftButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnLeftButtonDblClk(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnRightButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   LRESULT OnDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnEndSession(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   LRESULT OnMeasureItem(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnDrawItem(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   LRESULT OnMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnWindowPosChanging(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnDisplayChange(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT OnActivateApp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT OnMouseHover(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseLeave(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnNCHitTest(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   LRESULT OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   LRESULT OnHookMenuCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnPrepareHookMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnCheckMinToTray(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT OnCmdLanguageChange(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   static LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/);
};

extern VirtualDimension vdWindow;

#endif /* __VIRTUAL_DIMENSION_H__ */
