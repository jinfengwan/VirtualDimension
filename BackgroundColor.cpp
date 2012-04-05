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
#include "BackgroundColor.h"

BackgroundColor BackgroundColor::m_instance;

BackgroundColor::BackgroundColor()
{
   m_CurrentColor = m_SystemColor = GetSysColor(COLOR_DESKTOP);
}

BackgroundColor::~BackgroundColor()
{
   //Restore the initial color
   SetColor(m_SystemColor);
}

void BackgroundColor::SetColor(COLORREF col)
{
   static const INT P_COLOR_DESKTOP[] = {COLOR_DESKTOP};

   if (col == m_CurrentColor)
      return;

   if (SetSysColors(1, P_COLOR_DESKTOP, &col))
      m_CurrentColor = col;
}
