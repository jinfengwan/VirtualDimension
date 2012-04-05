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

#ifndef __ALWAYSONTOP_H__
#define __ALWAYSONTOP_H__

class AlwaysOnTop
{
public:
   AlwaysOnTop(HWND hWnd);
   ~AlwaysOnTop(void);

   void SetWindow(HWND hWnd);

   void SetAlwaysOnTop(bool onTop)  { if (onTop != m_ontop) ForceAlwaysOnTop(onTop); }
   void ForceAlwaysOnTop(bool ontop);
   bool IsAlwaysOnTop() const { return m_ontop; }

protected:
   HWND m_hWnd;
   bool m_ontop;
};

#endif /*__ALWAYSONTOP_H__*/
