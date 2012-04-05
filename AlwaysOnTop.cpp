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
#include "alwaysontop.h"

AlwaysOnTop::AlwaysOnTop(HWND hWnd): m_hWnd(hWnd)
{
   m_ontop = ((GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST);
}

AlwaysOnTop::~AlwaysOnTop(void)
{
}

/** Change the window which the object controls.
 * The previously selected state is applied to the new window immediatly and automatically.
 */
void AlwaysOnTop::SetWindow(HWND hWnd)
{
   if (m_hWnd != hWnd)
   {
      m_hWnd = hWnd;
      ForceAlwaysOnTop(m_ontop);
   }
}

/** Make a window on-top (or not).
 * This function is used to change the always-on-top state of the window. The change is 
 * performed immediatly. The SetAlwaysOnTop() should be used instead whenever possible, for better
 * performance, as it does not do anything if it is not needed.
 */
void AlwaysOnTop::ForceAlwaysOnTop(bool onTop)
{
   m_ontop = onTop;

   if (onTop)
      SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
   else
      SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}
