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

#ifndef __EXPLORERWRAPPER_H__
#define __EXPLORERWRAPPER_H__

#include <wininet.h>
#include <shlobj.h>
#include "FastWindow.h"
#include "PlatformHelper.h"

class ExplorerWrapper
{
public:
   ExplorerWrapper(FastWindow * wnd);
   ~ExplorerWrapper(void);

   inline void ShowWindowInTaskbar(HWND hWnd)
   {
#ifdef HIDEWINDOW_COMINTERFACE
      m_tasklist->AddTab(hWnd);
#else
      SendMessage(m_hWndTasklist, m_ShellhookMsg, HSHELL_WINDOWCREATED, (LPARAM)hWnd);
#endif
   }

   inline void HideWindowInTaskbar(HWND hWnd)
   {
#ifdef HIDEWINDOW_COMINTERFACE
      m_tasklist->DeleteTab(hWnd);
#else
      PostMessage(m_hWndTasklist, m_ShellhookMsg, HSHELL_WINDOWDESTROYED, (LPARAM)hWnd);
#endif
   }

	bool BindActiveDesktop();
   inline bool HasActiveDesktop() const        { return m_pActiveDesktop!=NULL; }
   inline IActiveDesktop * GetActiveDesktop()  { return m_pActiveDesktop; }

protected:
   LRESULT OnTaskbarRestart(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
   void BindTaskbar();

   /** Pointer to the COM ActiveDesktop interface.
    * Used for getting/changing wallpaper
    */
   IActiveDesktop * m_pActiveDesktop;

#ifdef HIDEWINDOW_COMINTERFACE
   /** Pointer to the COM taskbar interface.
   * This interface is used to add/remove the icons from the taskbar, when showing/hiding
   */
   ITaskbarList* m_tasklist;
#else
   HWND m_hWndTasklist;
   UINT m_ShellhookMsg;
#endif
};

extern ExplorerWrapper * explorerWrapper;

#endif /*__EXPLORERWRAPPER_H__*/
