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
#include "window.h"
#include "VirtualDimension.h"
#include "Settings.h"
#include "Locale.h"

#ifndef TBM_SETBUDDY
#define TBM_SETBUDDY (WM_USER+32)
#endif

void FormatTransparencyLevel(HWND hWnd, int level);

void Window::OpenSettings(Settings::Window &settings, bool create)
{
   settings.Open(m_className, create);
}

LRESULT CALLBACK Window::PropertiesProc(HWND hDlg, UINT message, WPARAM /*wParam*/, LPARAM lParam)
{
   Window * self;

	switch (message)
	{
	case WM_INITDIALOG:
      {
         LPPROPSHEETPAGE lpPage = (LPPROPSHEETPAGE)lParam;
         self = (Window*)lpPage->lParam;

         //Store the pointer to the window, for futur use
         SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)self);

         //Perform initialization
         return TRUE;
      }

   case WM_NOTIFY:
      LPNMHDR pnmh = (LPNMHDR) lParam;
      switch (pnmh->code)
      {
      case PSN_KILLACTIVE:
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, FALSE);
         return TRUE;

      case PSN_APPLY:
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
         return TRUE;
      }
      break;
	}
	return FALSE;
}

void Window::OnInitSettingsDlg(HWND hDlg)
{
   HWND hWnd;
   bool supportTransparency;   //is transparency supported on the platform ?
   Settings settings;

   supportTransparency = Transparency::IsTransparencySupported();

   //Setup the transparency slider and associated controls
   hWnd = GetDlgItem(hDlg, IDC_TRANSP_SLIDER);
   SendMessage(hWnd, TBM_SETRANGE, FALSE, MAKELONG(0,255));
   SendMessage(hWnd, TBM_SETTICFREQ, 16, 0);
   SendMessage(hWnd, TBM_SETBUDDY, TRUE, (LPARAM)GetDlgItem(hDlg, IDC_TRANSP_STATIC1));
   SendMessage(hWnd, TBM_SETBUDDY, FALSE, (LPARAM)GetDlgItem(hDlg, IDC_TRANSP_STATIC2));
   SendMessage(hWnd, TBM_SETPOS, TRUE, GetTransparencyLevel());
   EnableWindow(hWnd, supportTransparency);

   hWnd = GetDlgItem(hDlg, IDC_TRANSP_DISP);
   FormatTransparencyLevel(hWnd, GetTransparencyLevel());
   EnableWindow(hWnd, supportTransparency);

   EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC), supportTransparency);
   EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC1), supportTransparency);
   EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC2), supportTransparency);

   hWnd = GetDlgItem(hDlg, IDC_ENABLETRANSP_CHECK);
   EnableWindow(hWnd, supportTransparency);
   SendMessage(hWnd, BM_SETCHECK, IsTransparent() ? BST_CHECKED : BST_UNCHECKED, 0);

   //Setup always on top
   hWnd = GetDlgItem(hDlg, IDC_ONTOP_CHECK);
   SendMessage(hWnd, BM_SETCHECK, IsAlwaysOnTop() ? BST_CHECKED : BST_UNCHECKED, 0);

   //Setup minimize to tray
   hWnd = GetDlgItem(hDlg, IDC_MINTOTRAY_CHECK);
   SendMessage(hWnd, BM_SETCHECK, IsMinimizeToTray() ? BST_CHECKED : BST_UNCHECKED, 0);

   //Setup on all desktops
   hWnd = GetDlgItem(hDlg, IDC_ALLDESKS_CHECK);
   SendMessage(hWnd, BM_SETCHECK, IsOnAllDesktops() ? BST_CHECKED : BST_UNCHECKED, 0);
}

void Window::EraseSettings()
{
   Settings s;
   Settings::Window settings(&s);

   OpenSettings(settings, false);
   if (settings.IsValid())
      settings.Destroy();
}

void Window::SaveSettings()
{
   Settings s;
   Settings::Window settings(&s);

   OpenSettings(settings, true);
   if (!settings.IsValid())
   {
      locMessageBox(vdWindow, IDS_AUTOSETTINGS_ERROR, IDS_ERROR, MB_OK|MB_ICONERROR);
      return;
   }

   settings.SaveSetting(Settings::Window::AlwaysOnTop, IsAlwaysOnTop());
   settings.SaveSetting(Settings::Window::MinimizeToTray, IsMinimizeToTray());
   settings.SaveSetting(Settings::Window::OnAllDesktops, IsOnAllDesktops());
   settings.SaveSetting(Settings::Window::EnableTransparency, IsTransparent());
   settings.SaveSetting(Settings::Window::TransparencyLevel, GetTransparencyLevel());

   if (m_autopos || m_autosize)
   {
      RECT rect;

      if (GetWindowRect(m_hWnd, &rect))
         settings.SaveSetting(Settings::Window::WindowPosition, &rect);
   }
   if (m_autodesk && GetDesk())
      settings.SaveSetting(Settings::Window::DesktopIndex, GetDesk()->GetIndex());
}

void Window::OnApplySettingsBtn(HWND hDlg)
{
   bool res;
   HWND hWnd;

   //Apply always on top
   hWnd = GetDlgItem(hDlg, IDC_ONTOP_CHECK);
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   SetAlwaysOnTop(res);

   //Apply minimize to tray
   hWnd = GetDlgItem(hDlg, IDC_MINTOTRAY_CHECK);
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   SetMinimizeToTray(res);

   //Apply on all desktops
   hWnd = GetDlgItem(hDlg, IDC_ALLDESKS_CHECK);
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   SetOnAllDesktops(res);

   //Apply transparency level
   hWnd = GetDlgItem(hDlg, IDC_TRANSP_SLIDER);
   SetTransparencyLevel((unsigned char)SendMessage(hWnd, TBM_GETPOS, 0, 0));

   //Apply enable transparency
   hWnd = GetDlgItem(hDlg, IDC_ENABLETRANSP_CHECK);
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   SetTransparent(res);
}

LRESULT CALLBACK Window::SettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   Window * self;

	switch (message)
	{
	case WM_INITDIALOG:
      {
         LPPROPSHEETPAGE lpPage = (LPPROPSHEETPAGE)lParam;
         self = (Window*)lpPage->lParam;

         //Store the pointer to the window, for futur use
         SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)self);

         //Perform initialization
         self->OnInitSettingsDlg(hDlg);
		   return TRUE;
      }

   case WM_COMMAND:
      PropSheet_Changed(GetParent(hDlg), hDlg);
      break;

   case WM_HSCROLL:
      {
         int pos;
         switch(LOWORD(wParam))
         {
         case TB_THUMBPOSITION:
         case TB_THUMBTRACK:
            pos = HIWORD(wParam);
            break;
         case TB_BOTTOM :
         case TB_ENDTRACK:
         case TB_LINEDOWN:
         case TB_LINEUP:
         case TB_PAGEDOWN:
         case TB_PAGEUP:
         case TB_TOP :
         default:
            pos = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
            break;
         }
         FormatTransparencyLevel(GetDlgItem(hDlg, IDC_TRANSP_DISP), pos);
         PropSheet_Changed(GetParent(hDlg), hDlg);
      }
      break;

   case WM_NOTIFY:
      LPNMHDR pnmh = (LPNMHDR) lParam;
      switch (pnmh->code)
      {
      case PSN_KILLACTIVE:
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, FALSE);
         return TRUE;

      case PSN_APPLY:
         self = (Window *)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnApplySettingsBtn(hDlg);
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
         return TRUE;
      }
      break;
	}
	return FALSE;
}

void Window::OnInitAutoSettingsDlg(HWND hDlg)
{
   Settings s;
   Settings::Window settings(&s);
   HWND hWnd;
   AutoSettingsModes mode;

   //Find out which mode is used
   OpenSettings(settings, false);
   if (!settings.IsValid())
   {
      mode = ASS_DISABLED;
      hWnd = GetDlgItem(hDlg, IDC_DISABLE_RATIO);
   }
   else if (m_autoSaveSettings)
   {
      mode = ASS_AUTOSAVE;
      hWnd = GetDlgItem(hDlg, IDC_LASTSESSION_RATIO);
   }
   else
   {
      mode = ASS_SAVED;
      hWnd = GetDlgItem(hDlg, IDC_SAVED_RATIO);
   }

   //Check the appropriate button
   SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);

   //Check auto-set buttons
   hWnd = GetDlgItem(hDlg, IDC_AUTOSIZE_CHECK);
   SendMessage(hWnd, BM_SETCHECK, m_autosize ? BST_CHECKED : BST_UNCHECKED, 0);
   hWnd = GetDlgItem(hDlg, IDC_AUTOPOS_CHECK);
   SendMessage(hWnd, BM_SETCHECK, m_autopos ? BST_CHECKED : BST_UNCHECKED, 0);
   hWnd = GetDlgItem(hDlg, IDC_AUTODESK_CHECK);
   SendMessage(hWnd, BM_SETCHECK, m_autodesk ? BST_CHECKED : BST_UNCHECKED, 0);

   //Update the UI
   OnUpdateAutoSettingsUI(hDlg, mode);
}

void Window::OnApplyAutoSettingsBtn(HWND hDlg)
{
   HWND hWnd;
   Settings s;
   Settings::Window settings(&s);

   m_autoSaveSettings = false;

   //If auto-settings are disabled, remove the settings from registry
   if (SendMessage(GetDlgItem(hDlg, IDC_DISABLE_RATIO), BM_GETCHECK, 0, 0) == BST_CHECKED)
   {
      EraseSettings();
      return;
   }

   //Auto-set size ?
   hWnd = GetDlgItem(hDlg, IDC_AUTOSIZE_CHECK);
   m_autosize = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;

   //Auto-set position ?
   hWnd = GetDlgItem(hDlg, IDC_AUTOPOS_CHECK);
   m_autopos = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;

   //Auto-set desktop ?
   hWnd = GetDlgItem(hDlg, IDC_AUTODESK_CHECK);
   m_autodesk = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;

   //If loading last saved settings, and no settings already saved, do a save right now
   if (SendMessage(GetDlgItem(hDlg, IDC_SAVED_RATIO), BM_GETCHECK, 0, 0) == BST_CHECKED)
   {
      OpenSettings(settings, false);
      if (!settings.IsValid())
         SaveSettings();
   }
   else
      m_autoSaveSettings = true;

   //If not already open, try to open the Window's settings
   if (!settings.IsValid())
   {
      OpenSettings(settings, true); //If it does not exist, create it

      if (!settings.IsValid())
      {
         locMessageBox(hDlg, IDS_AUTOSETTINGS_ERROR, IDS_ERROR, MB_OK|MB_ICONERROR);
         return;
      }
   }

   //Save the auto-settings configuration to the registry
   settings.SaveSetting(Settings::Window::AutoSaveSettings, m_autoSaveSettings);
   settings.SaveSetting(Settings::Window::AutoSetSize, m_autosize);
   settings.SaveSetting(Settings::Window::AutoSetPos, m_autopos);
   settings.SaveSetting(Settings::Window::AutoSetDesk, m_autodesk);
}

void Window::OnUpdateAutoSettingsUI(HWND hDlg, AutoSettingsModes mode)
{
   EnableWindow(GetDlgItem(hDlg, IDC_SAVESETTINGS_BTN), mode == ASS_SAVED);

   EnableWindow(GetDlgItem(hDlg, IDC_AUTOSIZE_CHECK), mode != ASS_DISABLED);
   EnableWindow(GetDlgItem(hDlg, IDC_AUTOPOS_CHECK), mode != ASS_DISABLED);
   EnableWindow(GetDlgItem(hDlg, IDC_AUTODESK_CHECK), mode != ASS_DISABLED);
}

LRESULT CALLBACK Window::AutoSettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   Window * self;

	switch (message)
	{
	case WM_INITDIALOG:
      {
         LPPROPSHEETPAGE lpPage = (LPPROPSHEETPAGE)lParam;
         self = (Window*)lpPage->lParam;

         //Store the pointer to the window, for futur use
         SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)self);

         //Perform initialization
         self->OnInitAutoSettingsDlg(hDlg);
		   return TRUE;
      }

   case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case IDC_DISABLE_RATIO:
         self = (Window *)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnUpdateAutoSettingsUI(hDlg, ASS_DISABLED);
         PropSheet_Changed(GetParent(hDlg), hDlg);
         break;

      case IDC_LASTSESSION_RATIO:
         self = (Window *)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnUpdateAutoSettingsUI(hDlg, ASS_AUTOSAVE);
         PropSheet_Changed(GetParent(hDlg), hDlg);
         break;

      case IDC_SAVED_RATIO:
         self = (Window *)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnUpdateAutoSettingsUI(hDlg, ASS_SAVED);
         PropSheet_Changed(GetParent(hDlg), hDlg);
         break;

      case IDC_SAVESETTINGS_BTN:
         self = (Window *)GetWindowLongPtr(hDlg, DWLP_USER);
         self->SaveSettings();
         break;

      default:
         PropSheet_Changed(GetParent(hDlg), hDlg);
      }
      break;

   case WM_NOTIFY:
      LPNMHDR pnmh = (LPNMHDR) lParam;
      switch (pnmh->code)
      {
      case PSN_KILLACTIVE:
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, FALSE);
         return TRUE;

      case PSN_APPLY:
         self = (Window *)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnApplyAutoSettingsBtn(hDlg);
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
         return TRUE;
      }
      break;
	}
	return FALSE;
}

LRESULT CALLBACK Window::FilterSettingsProc(HWND /*hDlg*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   return FALSE;
}

void Window::DisplayWindowProperties()
{
   PROPSHEETPAGE pages[3];
   PROPSHEETHEADER propsheet;
	HINSTANCE hresinst = Locale::GetInstance();
	int page = 0;

   memset(&pages, 0, sizeof(pages));

   pages[page].dwSize = sizeof(PROPSHEETPAGE);
	pages[page].hInstance = hresinst;
   pages[page].dwFlags = PSP_DEFAULT;
   pages[page].pfnDlgProc = (DLGPROC)SettingsProc;
   pages[page].lParam = (LPARAM)this;
   pages[page].pszTemplate = MAKEINTRESOURCE(IDD_WINDOW_SETTINGS);\
   page++;

   pages[page].dwSize = sizeof(PROPSHEETPAGE);
   pages[page].hInstance = hresinst;
   pages[page].dwFlags = PSP_DEFAULT;
   pages[page].pfnDlgProc = (DLGPROC)AutoSettingsProc;
   pages[page].lParam = (LPARAM)this;
   pages[page].pszTemplate = MAKEINTRESOURCE(IDD_WINDOW_AUTOSETTINGS);
   page++;

   pages[page].dwSize = sizeof(PROPSHEETPAGE);
   pages[page].hInstance = hresinst;
   pages[page].dwFlags = PSP_DEFAULT;
   pages[page].pfnDlgProc = (DLGPROC)FilterSettingsProc;
   pages[page].lParam = (LPARAM)this;
   pages[page].pszTemplate = MAKEINTRESOURCE(IDD_WINDOW_FILTER);
   page++;

   memset(&propsheet, 0, sizeof(propsheet));
   propsheet.dwSize = sizeof(PROPSHEETHEADER);
   propsheet.dwFlags = PSH_PROPSHEETPAGE;
   propsheet.hwndParent = vdWindow;
   propsheet.hInstance = vdWindow;
   locGetString(propsheet.pszCaption, IDS_WINDOWPROPERTIES);
   propsheet.nPages = page;
   propsheet.ppsp = &pages[0];

   PropertySheet(&propsheet);
}
