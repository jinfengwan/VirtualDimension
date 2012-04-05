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

#ifndef __WINDOWSMANAGER_H__
#define __WINDOWSMANAGER_H__

#include <map>
#include <vector>
#include <list>
#include "Desktop.h"
#include "Window.h"
#include "ShellHook.h"
#include "WindowsList.h"
#include "HotkeyConfig.h"
#include "BalloonNotif.h"

using namespace std;

#define FIRST_WINDOW_MANAGER_TIMER     100
#define DELAYED_UPDATE_DELAY           1000 /*1sec*/

class WindowsManager
{
public:
   WindowsManager();
   ~WindowsManager(void);
   void PopulateInitialWindowsSet();

   void MoveWindow(HWND hWnd, Desktop* desk);
   Window* GetWindow(HWND hWnd);

   bool ConfirmKillWindow();
   bool IsConfirmKill() const         { return m_confirmKill; }
   void SetConfirmKill(bool confirm)  { m_confirmKill = confirm; }

   typedef WindowsList::Iterator Iterator;
   Iterator GetIterator()                   { return m_windows.begin(); }
   Iterator FirstWindow()                   { return m_windows.first(); }
   Iterator LastWindow()                    { return m_windows.last(); }

   Window * GetForegroundWindow();
   bool IsAutoSwitchDesktop() const         { return m_autoSwitch; }
   void SetAutoSwitchDesktop(bool autoSw)   { m_autoSwitch = autoSw; }
   bool IsShowAllWindowsInTaskList() const  { return m_allWindowsInTaskList; }
   void ShowAllWindowsInTaskList(bool all)  { m_allWindowsInTaskList = all; }
   bool IsIntegrateWithShell() const        { return m_integrateWithShell; }
   void SetIntegrateWithShell(bool integ);

   HWND GetPrevWindow(Window * wnd);

   void EnableAnimations();
   void DisableAnimations();

   void RemoveWindow(Window * win)          { OnWindowDestroyed(*win); }

   void ScheduleDelayedUpdate(Window * win);
   void CancelDelayedUpdate(Window * win);

protected:
   map<HWND, WindowsList::Node*> m_HWNDMap;
   WindowsList m_windows;

   list<Window*> m_zorder;

   typedef map<HWND, WindowsList::Node*>::iterator HWNDMapIterator;
   typedef list<Window*>::iterator ZOrderIterator;

   ShellHook m_shellhook;
   bool m_confirmKill;
   bool m_autoSwitch;
   bool m_allWindowsInTaskList;
   bool m_integrateWithShell;

   int m_iAnimate;
   LONG m_nbDisabledAnimations;

   LRESULT OnSettingsChange(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnStartOnDesktop(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnShellHookMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnWindowSizeMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   void OnWindowCreated(HWND hWnd);       //window has just been created
   void OnWindowDestroyed(HWND hWnd);     //window is going to be destroyed
   void OnWindowActivated(HWND hWnd);     //activation changed to another window
   void OnGetMinRect(HWND hWnd);          //window minimized/maximized
   void OnRedraw(HWND hWnd);              //window's title changed
   void OnSysMenu()                       { return; } //???
   void OnEndTask()                       { return; } //???
   void OnWindowReplaced(HWND)            { return; } //window has been replaced
   void OnWindowReplacing(HWND)           { return; } //window is being replaced
   void OnWindowFlash(HWND hWnd);         //window is flashing

   // Delayed window update
   //**************************************************************************
   typedef vector<Window *>::iterator DelayedUdateWndIterator;
   vector<Window *> m_delayedUpdateWndTab; //List of the windows which have requested delayed update
   vector<int> m_delayedUpdateNextTab;
   int m_firstFreeDelayedUpdateWndIdx;

   unsigned int AddDelayedUpdateWnd(Window * wnd);
   void RemoveDelayedUpdateWnd(unsigned int idx);
   static void CALLBACK OnDelayedUpdateTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

   // Various hotkeys
   //**************************************************************************
   class MoveWindowToNextDesktopEventHandler: public PersistentHotkey<Settings::MoveWindowToNextDesktopHotkey>
   {
   public:
      virtual void OnHotkey();
      virtual LPCSTR GetName() const	{ return "Move window to next desk"; }
   };

   class MoveWindowToPrevDesktopEventHandler: public PersistentHotkey<Settings::MoveWindowToPreviousDesktopHotkey>
   {
   public:
      virtual void OnHotkey();
      virtual LPCSTR GetName() const	{ return "Move window to previous desk"; }
   };

   class MoveWindowToDesktopEventHandler: public PersistentHotkey<Settings::MoveWindowToDesktopHotkey>
   {
   public:
      virtual void OnHotkey();
      virtual LPCSTR GetName() const	{ return "Move window to some desk"; }
   };

   class MaximizeHeightEventHandler: public PersistentHotkey<Settings::MaximizeHeightHotkey>
   {
   public:
      virtual void OnHotkey();
      virtual LPCSTR GetName() const   { return "Maximize height"; }
   };

   class MaximizeWidthEventHandler: public PersistentHotkey<Settings::MaximizeWidthHotkey>
   {
   public:
      virtual void OnHotkey();
      virtual LPCSTR GetName() const   { return "Maximize width"; }
   };

   class ToggleAlwaysOnTopEventHandler: public PersistentHotkey<Settings::AlwaysOnTopHotkey>
   {
   public:
		virtual void OnHotkey();
      virtual LPCSTR GetName() const   { return "Toggle always on top"; }
   };

   class ToggleTransparencyEventHandler: public PersistentHotkey<Settings::TransparencyHotkey>
   {
   public:
      virtual void OnHotkey();
      virtual LPCSTR GetName() const   { return "Toggle transparency"; }
   };

   MoveWindowToNextDesktopEventHandler m_moveToNextDeskEH;
   MoveWindowToPrevDesktopEventHandler m_moveToPrevDeskEH;
   MoveWindowToDesktopEventHandler m_moveToDesktopEH;
   MaximizeHeightEventHandler m_maximizeHeightEH;
   MaximizeWidthEventHandler m_maximizeWidthEH;
   ToggleAlwaysOnTopEventHandler m_toggleAlwaysOnTopEH;
   ToggleTransparencyEventHandler m_toggleTransparencyEH;

   static BOOL CALLBACK ListWindowsProc( HWND hWnd, LPARAM lParam );
};

extern WindowsManager * winMan;

#endif /*__WINDOWSMANAGER_H__*/
