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

#include "stdafx.h"
#include "VirtualDimension.h"
#include "settings.h"
#include "desktopmanager.h"
#include "Commdlg.h"
#include "PlatformHelper.h"
#include "Desktop.h"
#include "HotKeyControl.h"
#include "Locale.h"

Desktop::DesktopProperties::DesktopProperties(Desktop* desktop): m_desk(desktop), m_picture(NULL)
{
   return;
}

Desktop::DesktopProperties::~DesktopProperties()
{
   //Free the image, if any
   if (m_picture)
      m_picture->Release();
}

void Desktop::DesktopProperties::InitDialog(HWND hDlg)
{
   HWND hWnd;

   //Setup the color
   m_bgColor = m_desk->GetBackgroundColor();

   //Setup the wallpaper
   hWnd = GetDlgItem(hDlg, IDC_WALLPAPER);
   SendMessage(hWnd, EM_LIMITTEXT, (WPARAM)MAX_PATH, (LPARAM)0);
   SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)m_desk->GetWallpaper());

   //Setup the desktop's name
   hWnd = GetDlgItem(hDlg, IDC_NAME);
   SendMessage(hWnd, EM_LIMITTEXT, (WPARAM)sizeof(m_desk->m_name), (LPARAM)0);
   SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)m_desk->GetText());

   SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_SETHOTKEY, (WPARAM)m_desk->GetHotkey(), 0);

   //Disable the APPLY button
   EnableWindow(GetDlgItem(hDlg, IDC_APPLY), FALSE);
}

bool Desktop::DesktopProperties::Apply(HWND hDlg)
{
   //Change desktop name
   TCHAR name[80];   
   GetDlgItemText(hDlg, IDC_NAME, name, sizeof(name));
   m_desk->Rename(name);

   //Change wallpaper
   m_desk->SetWallpaper(m_wallpaper);

   //Set hotkey
   m_desk->SetHotkey((int)SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_GETHOTKEY, 0, 0));

   //Set background color
   m_desk->SetBackgroundColor(m_bgColor);

   //Disable the APPLY button
   EnableWindow(GetDlgItem(hDlg, IDC_APPLY), FALSE);

   return true;
}

void Desktop::DesktopProperties::OnWallpaperChanged(HWND hDlg, HWND ctrl)
{
   SendMessage(ctrl, WM_GETTEXT, MAX_PATH, (LPARAM)m_wallpaper);
   if (m_picture)
      m_picture->Release();
   if (stricmp(m_wallpaper, DESKTOP_WALLPAPER_DEFAULT) == 0)
      m_picture = PlatformHelper::OpenImage(WallPaper::GetDefaultWallpaper());
   else
      m_picture = PlatformHelper::OpenImage(m_wallpaper);
   InvalidateRect(GetDlgItem(hDlg, IDC_PREVIEW), NULL, TRUE);

   //Enable the APPLY button
   EnableWindow(GetDlgItem(hDlg, IDC_APPLY), TRUE);
}

void Desktop::DesktopProperties::OnBrowseWallpaper(HWND hDlg)
{
   OPENFILENAME ofn;
	String filter;

   // Reset text if a special mode is selected, to let the dialog open properly.
   if (*m_wallpaper == '<')
      *m_wallpaper = 0;

   // Initialize OPENFILENAME
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = hDlg;
   ofn.lpstrFile = m_wallpaper;
   ofn.nMaxFile = MAX_PATH;
	filter = Locale::GetInstance().GetString(IDS_PICTUREFILTER);
	filter.Replace('|', 0);
	ofn.lpstrFilter = filter;
   ofn.nFilterIndex = 1;
   ofn.lpstrFileTitle = NULL;
   ofn.nMaxFileTitle = 0;
   ofn.lpstrInitialDir = NULL;
   locGetString(ofn.lpstrTitle, IDS_SELECT_WALLPAPER);
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER /*| OFN_ENABLESIZING*/;

   if (GetOpenFileName(&ofn) == TRUE)
      SendMessage(GetDlgItem(hDlg, IDC_WALLPAPER), WM_SETTEXT, 0, (LPARAM)m_wallpaper);
   else
      SendMessage(GetDlgItem(hDlg, IDC_WALLPAPER), WM_GETTEXT, MAX_PATH, (LPARAM)m_wallpaper);
}

void Desktop::DesktopProperties::OnChooseWallpaper(HWND hDlg)
{
   RECT rect;
	HMENU hMenu = Locale::GetInstance().LoadMenu(IDM_WALLPAPER_CTXMENU);
   HMENU hPopupMenu = GetSubMenu(hMenu, 0);

   //Prepare the menu
   if (stricmp(m_wallpaper, DESKTOP_WALLPAPER_DEFAULT)==0)
      CheckMenuItem(hPopupMenu, IDC_DEFAULT_WALLPAPER, MF_CHECKED|MF_BYCOMMAND);
   else if (stricmp(m_wallpaper, DESKTOP_WALLPAPER_NONE)==0)
      CheckMenuItem(hPopupMenu, IDC_NO_WALLPAPER, MF_CHECKED|MF_BYCOMMAND);
   else
      CheckMenuItem(hPopupMenu, IDC_BROWSE_WALLPAPER, MF_CHECKED|MF_BYCOMMAND);

   //Display the menu right below the button
   GetWindowRect(GetDlgItem(hDlg, IDC_CHOOSE_WALLPAPER), &rect);
   TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_RIGHTALIGN, rect.right, rect.bottom, 0, hDlg, NULL);
   DestroyMenu(hMenu);
}

void Desktop::DesktopProperties::OnPreviewDrawItem(LPDRAWITEMSTRUCT lpDrawItem)
{
   if (m_picture)
   {
      PlatformHelper::CustomDrawIPicture(m_picture, lpDrawItem);
   }
   else
   {
		LPTSTR text;
		locGetString(text, IDS_NOIMAGE);
      FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, GetSysColorBrush(COLOR_BTNFACE));
      DrawText(lpDrawItem->hDC, text, -1, &lpDrawItem->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
   }
}

void Desktop::DesktopProperties::OnBgColorDrawItem(LPDRAWITEMSTRUCT lpDrawItem)
{
   LPRECT rect = &lpDrawItem->rcItem;
   HBRUSH brush = CreateSolidBrush(m_bgColor);

   FillRect(lpDrawItem->hDC, rect, brush);

   DeleteObject(brush);
}

void Desktop::DesktopProperties::SelectColor(HWND hDlg)
{
   CHOOSECOLOR cc;
   static COLORREF acrCustClr[16];

   ZeroMemory(&cc, sizeof(CHOOSECOLOR));
   cc.lStructSize = sizeof(CHOOSECOLOR);
   cc.hwndOwner = hDlg;
   cc.rgbResult = m_bgColor;
   cc.lpCustColors = acrCustClr;
   cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN;

   if (ChooseColor(&cc))
   {
      m_bgColor = cc.rgbResult;
      InvalidateRect(GetDlgItem(hDlg, IDC_BGCOLOR_BTN), NULL, FALSE);

      //Enable the APPLY button
      EnableWindow(GetDlgItem(hDlg, IDC_APPLY), TRUE);
   }
}

void Desktop::DesktopProperties::ResetWallpaper(HWND hDlg)
{
   SendMessage(GetDlgItem(hDlg, IDC_WALLPAPER), WM_SETTEXT, 0, (LPARAM)m_desk->GetWallpaper());
}

// Message handler for the desktop properties dialog box.
LRESULT CALLBACK Desktop::DeskProperties(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   DesktopProperties * self;

   switch (message)
	{
	case WM_INITDIALOG:
      self = new DesktopProperties((Desktop *)lParam);
      SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)self);
      self->InitDialog(hDlg);
		return TRUE;

	case WM_COMMAND:
      self = (DesktopProperties *)GetWindowLongPtr(hDlg, DWLP_USER);
		switch(LOWORD(wParam))
		{
      case IDC_APPLY:
         self->Apply(hDlg);
         break;

      case IDOK:
         self->Apply(hDlg);

      case IDCANCEL:
         delete self;
			EndDialog(hDlg, LOWORD(wParam)); //Return the last pressed button
			return TRUE;
 
      case IDC_CHOOSE_WALLPAPER: 
         if (HIWORD(wParam) == BN_CLICKED)
            self->OnChooseWallpaper(hDlg);
         else if (HIWORD(wParam) == BN_DOUBLECLICKED)
            self->OnBrowseWallpaper(hDlg);
        break;

      case IDC_DEFAULT_WALLPAPER:
         SendMessage(GetDlgItem(hDlg, IDC_WALLPAPER), WM_SETTEXT, 0, (LPARAM)DESKTOP_WALLPAPER_DEFAULT);
         break;

      case IDC_NO_WALLPAPER:
         SendMessage(GetDlgItem(hDlg, IDC_WALLPAPER), WM_SETTEXT, 0, (LPARAM)DESKTOP_WALLPAPER_NONE);
         break;

      case IDC_PREVIOUS_WALLPAPER:
         self->ResetWallpaper(hDlg);
         break;

      case IDC_BROWSE_WALLPAPER:
         self->OnBrowseWallpaper(hDlg);
         break;

      case IDC_WALLPAPER:
         switch(HIWORD(wParam))
         {
         case EN_CHANGE:
            self->OnWallpaperChanged(hDlg, (HWND)lParam);
            break;
         }
         break;

      case IDC_NAME:
         switch(HIWORD(wParam))
         {
         case EN_CHANGE:
            //Enable the APPLY button
            EnableWindow(GetDlgItem(hDlg, IDC_APPLY), TRUE);
            break;
         }
         break;

      case IDC_HOTKEY:
         switch(HIWORD(wParam))
         {
         case HKN_CHANGE:
            //Enable the APPLY button
            EnableWindow(GetDlgItem(hDlg, IDC_APPLY), TRUE);
            break;
         }
         break;

      case IDC_BGCOLOR_BTN:
         switch(HIWORD(wParam))
         {
         case STN_CLICKED:
            //Enable the APPLY button
            self->SelectColor(hDlg);
            break;
         }
         break;
      }
		break;

   case WM_DRAWITEM:
      switch(wParam)
      {
      case IDC_BGCOLOR_BTN:
         self = (DesktopProperties *)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnBgColorDrawItem((LPDRAWITEMSTRUCT)lParam);
         return TRUE;

      case IDC_PREVIEW:
         self = (DesktopProperties *)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnPreviewDrawItem((LPDRAWITEMSTRUCT)lParam);
         return TRUE;
      }
   }
	return FALSE;
}

bool Desktop::Configure(HWND hDlg)
{
	return DialogBoxParam(Locale::GetInstance(), MAKEINTRESOURCE(IDD_DESKTOPPROPS), hDlg, (DLGPROC)&DeskProperties, (LPARAM)this) == IDOK;
}
