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
#include "Window.h"
#include "HidingMethod.h"
#include "ExplorerWrapper.h"
#include "Window.h"
#include "WindowsManager.h"

void HidingMethod::SetWindowData(Window * wnd, int data)
{
   wnd->m_hidingMethodData = data;
}

int HidingMethod::GetWindowData(const Window * wnd)
{
   return wnd->m_hidingMethodData;
}


void HidingMethodHide::Attach(Window * wnd)
{
	if (!::IsWindowVisible(*wnd))
	{
		SetWindowData(wnd, HIDDEN);
		Show(wnd);
	}
	else
      SetWindowData(wnd, SHOWN); 
}

void HidingMethodHide::Show(Window * wnd)
{
   int iconic = GetWindowData(wnd) & 0x10;
   int state = GetWindowData(wnd) & 0xf;

   switch(state)
   {
   case HIDING:
   case SHOWING_TO_HIDE:
   case HIDDEN:
      SetWindowData(wnd, SHOWING | iconic);

      winMan->DisableAnimations();
      SetWindowPos(*wnd, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      if (!iconic)
         ShowOwnedPopups(*wnd, TRUE);
      winMan->EnableAnimations();
      break;
   }
}

void HidingMethodHide::Hide(Window * wnd)
{
   int state = GetWindowData(wnd) & 0xf;
   int iconic = GetWindowData(wnd) & 0x10;

   switch(state)
   {
   case SHOWN:
      iconic = ::IsIconic(*wnd) ? 0x10 : 0;
   case SHOWING:
   case HIDING_TO_SHOW:
      SetWindowData(wnd, HIDING | iconic);
      
      winMan->DisableAnimations();
      //need to hide the parent window first or hiding owned windows causes an activate of the parent (theory, didn't confirm)
      SetWindowPos(*wnd, NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      if (!iconic)
         ShowOwnedPopups(*wnd, FALSE);
      winMan->EnableAnimations();
      break;
   }
}

bool HidingMethodHide::CheckCreated(Window * wnd)
{
   int data = GetWindowData(wnd);
   bool res;

   switch(data&0xf)
   {
   case SHOWING:
      SetWindowData(wnd, SHOWN|(data&0x10));
      res = false;
      break;

   case SHOWING_TO_HIDE:
      SetWindowData(wnd, SHOWN|(data&0x10));
      Hide(wnd);
      res = false;
      break;

   default:
      res = true;
      break;
   }

   return res;
}

bool HidingMethodHide::CheckDestroyed(Window * wnd)
{
   int data = GetWindowData(wnd);
   bool res;

   switch(data&0xf)
   {
   case HIDING:
      SetWindowData(wnd, HIDDEN|(data&0x10));
      res = false;
      break;

   case HIDING_TO_SHOW:
      SetWindowData(wnd, HIDDEN|(data&0x10));
      Show(wnd);
      res = false;
      break;

   default:
      res = true;
      break;
   }

   return res;
}

bool HidingMethodHide::CheckSwitching(const Window * wnd)
{
   int data = GetWindowData(wnd) & 0xf;
   return data != SHOWN && data != HIDDEN;
}


void HidingMethodMinimize::Attach(Window * wnd)
{
	 LONG_PTR style = GetWindowLongPtr(*wnd, GWL_EXSTYLE);
	 if (style & WS_EX_TOOLWINDOW)
	 {
        SetWindowData(wnd, (style & ~WS_EX_TOOLWINDOW) | WS_EX_APPWINDOW);
        Show(wnd);
	 }
}

void HidingMethodMinimize::Show(Window * wnd)
{
   LONG_PTR oldstyle = GetWindowData(wnd) & 0x7fffffff;

   //Restore the window's style
   if (oldstyle != GetWindowLongPtr(*wnd, GWL_EXSTYLE))
   {
      SetWindowLongPtr(*wnd, GWL_EXSTYLE, oldstyle);
      SetWindowPos(*wnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
   }

   //Show the icon
   explorerWrapper->ShowWindowInTaskbar(*wnd);

   //Restore the application if needed
   if (!(GetWindowData(wnd) >> 31))
   {
      winMan->DisableAnimations();
      ::ShowWindow(*wnd, SW_SHOWNOACTIVATE);
      winMan->EnableAnimations();

      SetWindowPos(*wnd, winMan->GetPrevWindow(wnd), 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
   }
}

void HidingMethodMinimize::Hide(Window * wnd)
{
   LONG_PTR oldstyle = GetWindowLongPtr(*wnd, GWL_EXSTYLE) & 0x7fffffff;

   //Minimize the application
   int iconic = wnd->IsIconic() ? 1 : 0;
   if (!iconic)
   {
      winMan->DisableAnimations();
      ::ShowWindow(*wnd, SW_SHOWMINNOACTIVE);
      winMan->EnableAnimations();
   }

   SetWindowData(wnd, oldstyle | (iconic << 31));

   //Hide the icon
   explorerWrapper->HideWindowInTaskbar(*wnd);

   //disable the window so that it does not appear in task list
   if (!winMan->IsShowAllWindowsInTaskList())
   {
      LONG_PTR style = (oldstyle & ~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW;
      SetWindowLongPtr(*wnd, GWL_EXSTYLE, style);
      SetWindowPos(*wnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
   }
}


void HidingMethodMove::Attach(Window * wnd)
{
	 LONG_PTR style = GetWindowLongPtr(*wnd, GWL_EXSTYLE);
	 if (style & WS_EX_TOOLWINDOW)
	 {
        SetWindowData(wnd, (style & ~WS_EX_TOOLWINDOW) | WS_EX_APPWINDOW);
        Show(wnd);
	 }
}

void HidingMethodMove::Show(Window * wnd)
{
   RECT aPosition;
   GetWindowRect(*wnd, &aPosition);

   // Restore the window mode
   SetWindowLong(*wnd, GWL_EXSTYLE, GetWindowData(wnd));  

   // Notify taskbar of the change
   explorerWrapper->ShowWindowInTaskbar(*wnd);

   // Bring back to visible area, SWP_FRAMECHANGED makes it repaint 
   SetWindowPos(*wnd, 0, aPosition.left, - (100 + aPosition.bottom )/2, 
                0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE ); 

   // Notify taskbar of the change
   explorerWrapper->ShowWindowInTaskbar(*wnd);
}

void HidingMethodMove::Hide(Window * wnd)
{
   RECT aPosition;
   GetWindowRect(*wnd, &aPosition);

   // Move the window off visible area
   SetWindowPos(*wnd, 0, aPosition.left, - aPosition.top - aPosition.bottom - 100, 
                0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

   // This removes window from taskbar and alt+tab list
   LONG_PTR style = GetWindowLongPtr(*wnd, GWL_EXSTYLE);
   SetWindowLongPtr(*wnd, GWL_EXSTYLE, 
                    style & (~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW);
   SetWindowData(wnd, style);

   // Notify taskbar of the change
   explorerWrapper->HideWindowInTaskbar(*wnd);
}
