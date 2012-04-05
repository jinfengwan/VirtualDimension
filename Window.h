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

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "desktop.h"
#include <shlobj.h>
#include "TrayIconsManager.h"
#include "Transparency.h"
#include "AlwaysOnTop.h"
#include "HidingMethod.h"
#include "SharedMenuBuffer.h"
#include "BalloonNotif.h"


class Window: public ToolTip::Tool, public TrayIconsManager::TrayIconHandler, public AlwaysOnTop
{
   friend class HidingMethod;

public:
   /** Constructor.
    * Builds a Window object from the handle of a window. Settings specific to this window are loaded
    * from registry, if any, and applied. Else, default settings are used.
    * If shell integration is enabled, the window gets hooked at this time.
    */
   Window(HWND hWnd);

   /** Destructor.
    * Performs cleanup: settings are saved to the registry if needed, the window is unhooked and
    * memory/handles are released.
    */
   virtual ~Window();

   /** Move the window to the specified desktop.
    * This function allows to specify the desktop on which the window can be seen.
    * By specifying NULL, the window will be seen on all desktops.
    *
    * @param desk Pointer to the desktop on to which the window belongs (ie, on which the
    * window is displayed), or NULL to make the window always visible (ie, on all desktops)
    */
   void MoveToDesktop(Desktop * desk);

   /** Tell if the window can be seen on the specified desktop.
    * This function returns a boolean, indicating if the window is displayed on the
    * specified desktop. By specifying the NULL desktop, the caller can easily find out
    * if the window can be seen on all desktops.
    * This does not give any information concerning the window state or whatsoever. It
    * simply tells if the windows is "present" on the specified desktop.
    *
    * @param desk Pointer to the desktop on which one wants to know if the window is
    * displayed, or NULL to find out if the window is displayed on all desktops.
    * @retval true if the window is visible on the specified desktop
    * @retval false if the window is not visible on the specified desktop
    * @see IsOnCurrentDesk, GetDesk
    */
   bool IsOnDesk(Desktop * desk) const        { return (m_desk == NULL) || (m_desk == desk); }

   /** Tell if the window is visible on the current desktop.
    * This function work exactly as IsOnDesk(), except that it checks only for the current
    * desktop.
    *
    * @retval true if the window is visible on the specified desktop
    * @retval false if the window is not visible on the specified desktop
    * @see IsOnDesk, GetDesk
    */
   bool IsOnCurrentDesk() const;

   /** Get the desktop on which the window is present.
    * If the window is present on all desktops, the return value is NULL.
    *
    * @return Pointer to the Desktop which the window belongs to, or NULL if the window is
    * visible on all desktops.
    * @see IsOnDesk, IsOnCurrentDesk
    */
   Desktop * GetDesk() const                  { return m_desk; }

   /** Builds the context menu associated with the window.
    * This function creates and initializes a popup menu that can used to perform a variety of actions
    * on the window:
    *   - all regular system menu actions (move, size, close...)
    *   - maximize height/width
    *   - kill
    *   - enable minimize to tray, transparency, present on all desktops
    *   - change the desktop
    *   - display the properties dialog
    *
    * After the menu is displayed, the action can be performed by calling the OnMenuItemSelected method
    * with both the handle of the menu and the id of the selected command. After all this is done, the
    * menu should be destroyed.
    *
    * Notice that it is the responsibility of the caller to do so: the method does not display the menu
    * nor enable the user to select anything. It simply creates the menu in memory.
    *
    * @return Handle to the newly created menu
    * @see OnMenuItemSelected
    */
   HMENU BuildMenu();
   bool PrepareSysMenu(HANDLE filemapping);
   void OnMenuItemSelected(HMENU menu, int cmdId);

   inline void ShowWindow();
   inline void HideWindow();
   inline bool IsHidden() const               { return m_hidden; }
   inline bool CheckCreated();
   inline bool CheckDestroyed();

   bool IsMinimizeToTray() const              { return m_MinToTray; }
   void ToggleMinimizeToTray();
   void SetMinimizeToTray(bool totray);
   bool IsIconic() const                      { return IsHidden() ? m_iconic : (::IsIconic(m_hWnd) ? true:false); }
   bool IsInTray() const                      { return IsMinimizeToTray() && IsIconic(); }
   void Restore();

   void ToggleOnTop();

   bool IsOnAllDesktops() const               { return IsOnDesk(NULL); }
   void SetOnAllDesktops(bool onall);
   void ToggleAllDesktops();
   void Activate();
   void Minimize();
   void Maximize();
   void MaximizeHeight();
   void MaximizeWidth();
   void Kill();

   void DisplayWindowProperties();

   bool IsTransparent() const                 { return m_transp.GetTransparencyLevel() != TRANSPARENCY_DISABLED; }
   void SetTransparent(bool transp);
   void ToggleTransparent();
   unsigned char GetTransparencyLevel() const { return m_transpLevel; }
   void SetTransparencyLevel(unsigned char level);

   operator HWND()                            { return m_hWnd; }

   HICON GetIcon(void);
   char * GetText()
   {
      GetWindowText(m_hWnd, m_name, sizeof(m_name)/sizeof(char));
      return m_name;
   }
   void GetRect(LPRECT /*rect*/)  { return; }

   void Hook();
   void UnHook();

   inline HWND GetOwnedWindow() const         { return m_hOwnedWnd; }
   inline static HWND GetOwnedWindow(HWND hWnd);

   inline bool IsSwitching() const            { return m_switching || m_hidingMethod->CheckSwitching(this); }
   inline void SetSwitching(bool on)          { m_switching = on; }

	inline bool IsMoving() const					 { return m_moving; }
	inline void SetMoving(bool moving)			 { m_moving = moving; }

   inline bool CheckExists() const            { return IsWindow(m_hWnd) != 0; }

   void OnDelayUpdate();

   static void SetTag(HWND hWnd, int val)     { SetProp(hWnd, MAKEINTATOM(s_VDPropertyTag), (HANDLE)(val+1)); }
   static void RemTag(HWND hWnd)              { RemoveProp(hWnd, MAKEINTATOM(s_VDPropertyTag)); }
   static bool HasTag(HWND hWnd)              { return GetProp(hWnd, MAKEINTATOM(s_VDPropertyTag)) != NULL; }
   static int GetTag(HWND hWnd)               { return (int)GetProp(hWnd, MAKEINTATOM(s_VDPropertyTag)) - 1; }

   void FlashWindow(void);
   void UnFlashWindow(void);
   bool IsWindowFlashing(void)                { return m_BallonMsg ? true : false; }

protected:
   LRESULT OnTrayIconMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   void OnContextMenu();
   void InsertMenuItem(HMENU menu, bool checked, HANDLE bmp, UINT id, UINT uIdStr);
	void InsertMenuInfo(SharedMenuBuffer& menuinfo, UINT id, UINT uIdStr, bool checked);
   HANDLE LoadBmpRes(int id);

   enum AutoSettingsModes {
      ASS_DISABLED,
      ASS_AUTOSAVE,
      ASS_SAVED
   };

   void OpenSettings(Settings::Window &settings, bool create=false);
   void EraseSettings();
   void SaveSettings();

   static LRESULT CALLBACK PropertiesProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

   void OnInitSettingsDlg(HWND hDlg);
   void OnApplySettingsBtn(HWND hDlg);
   static LRESULT CALLBACK SettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

   void OnInitAutoSettingsDlg(HWND hDlg);
   void OnApplyAutoSettingsBtn(HWND hDlg);
   void OnUpdateAutoSettingsUI(HWND hDlg, AutoSettingsModes mode);
   static LRESULT CALLBACK AutoSettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

   static LRESULT CALLBACK FilterSettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

   static void OnFlashBallonClick(BalloonNotification::Message msg, int data);

   HWND m_hWnd;
   HWND m_hOwnedWnd;
   Desktop * m_desk;
   bool m_MinToTray;
   bool m_iconic;
   char m_name[255];
   LONG_PTR m_style;
   bool m_setStyle;

   Transparency m_transp;
   unsigned char m_transpLevel;

   bool m_autoSaveSettings;
   bool m_autosize;
   bool m_autopos;
   bool m_autodesk;

   TCHAR m_className[30];
   HICON m_hIcon;
   HICON m_hDefaulIcon;

   BalloonNotification::Message m_BallonMsg;

   HINSTANCE m_HookDllHandle;
   DWORD m_dwProcessId;

   bool m_switching;
	bool m_moving;

   bool m_hidden;
   HidingMethod * m_hidingMethod;
   int m_hidingMethodData;
   HANDLE m_hHideMutex;		//used to prevent hiding & showing at the same time if switching quickly to a desktop and back to the first

   static HidingMethodHide       s_hider_method;
   static HidingMethodMinimize   s_minimizer_method;
   static HidingMethodMove       s_mover_method;

   static HidingMethod* s_hiding_methods[];

   static const ATOM s_VDPropertyTag;
};

HWND Window::GetOwnedWindow(HWND hWnd)
{
   HWND owned = GetWindow(hWnd, 6/*GW_ENABLEDPOPUP*/);
   return owned ? owned : hWnd;
}

void Window::ShowWindow()
{
   WaitForSingleObject(m_hHideMutex, INFINITE);
   if (m_hidden)
   {
      m_hidingMethod->Show(this);
      m_hidden = false;
   }
   ReleaseMutex(m_hHideMutex);
}

void Window::HideWindow()
{
   WaitForSingleObject(m_hHideMutex, INFINITE);
   if (!m_hidden)
   {
      m_hidingMethod->Hide(this);
      m_hidden = true;
   }
   ReleaseMutex(m_hHideMutex);
}

bool Window::CheckCreated()
{
   bool res;

   WaitForSingleObject(m_hHideMutex, INFINITE);
   res = m_hidingMethod->CheckCreated(this);
   ReleaseMutex(m_hHideMutex);

   return res;
}

bool Window::CheckDestroyed()
{
   bool res;

   WaitForSingleObject(m_hHideMutex, INFINITE);
   res = m_hidingMethod->CheckDestroyed(this);
   ReleaseMutex(m_hHideMutex);

   return res;
}

#endif /*__WINDOW_H__*/
