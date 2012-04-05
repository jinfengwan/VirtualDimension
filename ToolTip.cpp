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
#include "tooltip.h"
#include "virtualdimension.h"
#include "settings.h"

ToolTip::ToolTip(HWND hWnd): m_hOwnerWnd(hWnd)
{
   Settings settings;

   //Create the control
   m_hWnd = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
                           WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                           hWnd, NULL, vdWindow, NULL);

   //Load from the registry to find out whether to enable tooltips or not
   EnableTooltips(settings.LoadSetting(Settings::EnableToolTips));
}

ToolTip::~ToolTip(void)
{
   Settings settings;

   //Save to the registry whether to enable tooltips or not
   settings.SaveSetting(Settings::EnableToolTips, m_enabled);

   //Destroy the tooltip window
   DestroyWindow(m_hWnd);
}

void ToolTip::SetTool(Tool * tool, LPRECT rect)
{
   TOOLINFO ti;
   LRESULT update;

   ti.cbSize = sizeof(TOOLINFO);
   ti.hwnd = m_hOwnerWnd;
   ti.uId = (UINT_PTR)tool;
   ti.lpszText = NULL;

   update = SendMessage(m_hWnd, TTM_GETTOOLINFO, 0, (LPARAM) (LPTOOLINFO) &ti);

   ti.uFlags = TTF_SUBCLASS;
   ti.hinst = vdWindow;
   ti.lpszText = tool->GetText();
   if (rect)
      ti.rect = *rect;
   else
      tool->GetRect(&ti.rect);

   if (update)
      SendMessage(m_hWnd, TTM_SETTOOLINFO, 0, (LPARAM) (LPTOOLINFO) &ti);
   else
      SendMessage(m_hWnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
}

void ToolTip::UnsetTool(Tool * tool)
{
   TOOLINFO ti;

   ti.cbSize = sizeof(TOOLINFO);
   ti.hwnd = m_hOwnerWnd;
   ti.uId = (UINT_PTR)tool;

   SendMessage(m_hWnd, TTM_DELTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
}

void ToolTip::EnableTooltips(bool enable)
{
   m_enabled = enable;

   ShowTooltips();
}

void ToolTip::ShowTooltips(bool visible)
{
   if (visible)
      SendMessage(m_hWnd, TTM_ACTIVATE, (WPARAM)(BOOL)m_enabled, 0);
   else
      SendMessage(m_hWnd, TTM_ACTIVATE, 0, 0);
}
