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
#include "DesktopManager.h"
#include "WindowsManager.h"
#include "transparency.h"
#include "OnScreenDisplay.h"
#include <string.h>
#include <prsht.h>
#include <assert.h>
#include "HotkeyConfig.h"
#include "ApplicationListDlg.h"
#include "Locale.h"

extern char desk_name[80];
extern char desk_wallpaper[256];
extern int desk_hotkey;
LRESULT CALLBACK DeskProperties(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void FormatTransparencyLevel(HWND hWnd, int level)
{
   char buffer[15];

   if (level == TRANSPARENCY_DISABLED)
	{
		char * disabled;
		locGetString(disabled, IDS_DISABLED);
		sprintf(buffer, "%3i (%s)", level, disabled);
	}
   else
      sprintf(buffer, "%3i", level);

   SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)buffer);
}

// Message handler for the global settings page.
LRESULT CALLBACK SettingsConfiguration(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
      {
         HWND hWnd;
         Settings settings;

         //Setup always on top
         CheckDlgButton(hDlg, IDC_ONTOP_CHECK, ontop->IsAlwaysOnTop() ? BST_CHECKED : BST_UNCHECKED);

         //Setup enable tray icon
         CheckDlgButton(hDlg, IDC_TRAYICON_CHECK, trayIcon->HasIcon() ? BST_CHECKED : BST_UNCHECKED);

         //Setup enable tooltips
         CheckDlgButton(hDlg, IDC_TOOLTIPS_CHECK, tooltip->IsEnabled() ? BST_CHECKED : BST_UNCHECKED);

         //Setup confirm killing
         CheckDlgButton(hDlg, IDC_CONFIRMKILL_CHECK, winMan->IsConfirmKill() ? BST_CHECKED : BST_UNCHECKED);

         //Setup close to tray
         hWnd = GetDlgItem(hDlg, IDC_CLOSETOTRAY_CHECK);
         EnableWindow(hWnd, trayIcon->HasIcon());
         SendMessage(hWnd, BM_SETCHECK, trayIcon->IsCloseToTray() ? BST_CHECKED : BST_UNCHECKED, 0);

         //Setup start with windows
         CheckDlgButton(hDlg, IDC_STARTWITHWINDOWS_CHECK, settings.LoadStartWithWindows() ? BST_CHECKED : BST_UNCHECKED);

         //Setup auto switch desktop
         CheckDlgButton(hDlg, IDC_AUTOSWITCHDESKTOP_CHECK, winMan->IsAutoSwitchDesktop() ? BST_CHECKED : BST_UNCHECKED);

         //Setup all windows in task list
         CheckDlgButton(hDlg, IDC_ALLWINDOWSINTASKLIST_CHECK, winMan->IsShowAllWindowsInTaskList() ? BST_CHECKED : BST_UNCHECKED);

			//Setup snap size
			SetDlgItemInt(hDlg, IDC_SNAPSIZE_EDIT, vdWindow.GetSnapSize(), FALSE);
			hWnd = GetDlgItem(hDlg, IDC_SNAPSIZE_SPIN);
			SendMessage(hWnd, UDM_SETRANGE32, 0, 100);

			//Setup auto hide delay
			SetDlgItemInt(hDlg, IDC_AUTOHIDEDELAY_EDIT, vdWindow.GetAutoHideDelay(), FALSE);
			hWnd = GetDlgItem(hDlg, IDC_AUTOHIDEDELAY_SPIN);
			SendMessage(hWnd, UDM_SETRANGE32, 0, 0xffffff);

         //Setup mouse warp
         CheckDlgButton(hDlg, IDC_MOUSEWARP_CHECK, mousewarp->IsWarpEnabled() ? BST_CHECKED : BST_UNCHECKED);
      }
		return TRUE;

   case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case IDC_TRAYICON_CHECK: {
         HRESULT res = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);
         HWND hWnd = GetDlgItem(hDlg, IDC_CLOSETOTRAY_CHECK);
         EnableWindow(hWnd, res);
         }break;

      case IDC_MOUSEWARPCFG_BTN:
         mousewarp->EnableWarp(IsDlgButtonChecked(hDlg, IDC_MOUSEWARP_CHECK) == BST_CHECKED);
         mousewarp->Configure(hDlg);
         CheckDlgButton(hDlg, IDC_MOUSEWARP_CHECK, mousewarp->IsWarpEnabled() ? BST_CHECKED : BST_UNCHECKED);
         break;
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
         {
            Settings settings;
            bool res;
            HWND hWnd;

            //Apply always on top
            hWnd = GetDlgItem(hDlg, IDC_ONTOP_CHECK);
            res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
            ontop->SetAlwaysOnTop(res);

            //Apply enable tray icon
            hWnd = GetDlgItem(hDlg, IDC_TRAYICON_CHECK);
            res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
            trayIcon->SetIcon(res);

            //Apply enable tooltips
            hWnd = GetDlgItem(hDlg, IDC_TOOLTIPS_CHECK);
            res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
            tooltip->EnableTooltips(res);

            //Apply confirm killing
            hWnd = GetDlgItem(hDlg, IDC_CONFIRMKILL_CHECK);
            res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
            winMan->SetConfirmKill(res);

            //Apply close to tray
            hWnd = GetDlgItem(hDlg, IDC_CLOSETOTRAY_CHECK);
            res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
            trayIcon->SetCloseToTray(res);

            //Apply start with windows
            hWnd = GetDlgItem(hDlg, IDC_STARTWITHWINDOWS_CHECK);
            res = SendMessage(hWnd, BM_GETCHECK, 0, 0) ==  BST_CHECKED ? true : false;
            settings.SaveStartWithWindows(res);

            //Apply auto switch desktop
            winMan->SetAutoSwitchDesktop(IsDlgButtonChecked(hDlg, IDC_AUTOSWITCHDESKTOP_CHECK) == BST_CHECKED);

            //Setup integrate with shell
            winMan->ShowAllWindowsInTaskList(IsDlgButtonChecked(hDlg, IDC_ALLWINDOWSINTASKLIST_CHECK) == BST_CHECKED);

				//Apply snap-size
				vdWindow.SetSnapSize(GetDlgItemInt(hDlg, IDC_SNAPSIZE_EDIT, NULL, FALSE));

				//Apply auto-hide delay
				vdWindow.SetAutoHideDelay(GetDlgItemInt(hDlg, IDC_AUTOHIDEDELAY_EDIT, NULL, FALSE));

            //Apply mouse warp
            mousewarp->EnableWarp(IsDlgButtonChecked(hDlg, IDC_MOUSEWARP_CHECK) == BST_CHECKED);

            //Apply succeeded
            SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
         }
         return TRUE;
      }
      break;
	}
	return FALSE;
}

// Message handler for the display settings page.
LRESULT CALLBACK DisplayConfiguration(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char * text;

	switch (message)
	{
	case WM_INITDIALOG:
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
         SendMessage(hWnd, TBM_SETPOS, TRUE, transp->GetTransparencyLevel());
         EnableWindow(hWnd, supportTransparency);

         hWnd = GetDlgItem(hDlg, IDC_TRANSP_DISP);
         FormatTransparencyLevel(hWnd, transp->GetTransparencyLevel());
         EnableWindow(hWnd, supportTransparency);

         EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC), supportTransparency);
         EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC1), supportTransparency);
         EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC2), supportTransparency);

         //Setup display mode
         UINT ratioItem;
         switch(deskMan->GetDisplayMode())
         {
         default:
         case DesktopManager::DM_PLAINCOLOR: ratioItem = IDC_PLAINCOLOR_RATIO; break;
         case DesktopManager::DM_PICTURE:    ratioItem = IDC_IMAGE_RATIO;      break;
         case DesktopManager::DM_SCREENSHOT: ratioItem = IDC_SCREENSHOT_RATIO; break;
         }
         CheckRadioButton(hDlg, IDC_PLAINCOLOR_RATIO, IDC_SCREENSHOT_RATIO, ratioItem);

         //Setup display mode options, by simulating a click
         SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(ratioItem,BN_CLICKED), (LPARAM)GetDlgItem(hDlg, ratioItem));
      }
		return TRUE;

   case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case IDC_PLAINCOLOR_RATIO:
			locGetString(text, IDS_PARAM_COLOR);
         SetDlgItemText(hDlg, IDC_EXTRAPARAM_BTN, text);
         EnableWindow(GetDlgItem(hDlg, IDC_EXTRAPARAM_BTN), TRUE);
         deskMan->SetDisplayMode(DesktopManager::DM_PLAINCOLOR);
         break;

      case IDC_IMAGE_RATIO:
			locGetString(text, IDS_PARAM_IMAGE);
         SetDlgItemText(hDlg, IDC_EXTRAPARAM_BTN, text);
         EnableWindow(GetDlgItem(hDlg, IDC_EXTRAPARAM_BTN), TRUE);
         deskMan->SetDisplayMode(DesktopManager::DM_PICTURE);
         break;

      case IDC_SCREENSHOT_RATIO:
			locGetString(text, IDS_PARAM_COLOR);
         SetDlgItemText(hDlg, IDC_EXTRAPARAM_BTN, text);
         EnableWindow(GetDlgItem(hDlg, IDC_EXTRAPARAM_BTN), FALSE);
         deskMan->SetDisplayMode(DesktopManager::DM_SCREENSHOT);
         break;

      case IDC_EXTRAPARAM_BTN:
         deskMan->ChooseBackgroundDisplayModeOptions(hDlg);
         break;

      case IDC_FONT_BTN:
         deskMan->ChoosePreviewWindowFont(hDlg);
         break;
      }
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
         transp->SetTransparencyLevel((unsigned char)pos);
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
         //Apply succeeded
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
         return TRUE;
      }
      break;
	}
	return FALSE;
}

// Message handler for desktop configuration page.
LRESULT CALLBACK DeskConfiguration(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
      {
         Desktop * desk;
         HWND hWnd;

         //Fill the listbox control with the various desks
         hWnd = GetDlgItem(hDlg, IDC_DESK_LIST);
         for(desk = deskMan->GetFirstDesktop();
             desk != NULL;
             desk = deskMan->GetNextDesktop())
         {
            LRESULT index = SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)desk->GetText());
            SendMessage(hWnd, LB_SETITEMDATA, index, (LPARAM)desk);
         }

         //Disable the desktop order spin buttons, as well as remove and setup buttons
         EnableWindow(GetDlgItem(hDlg, IDC_DESK_SPIN), FALSE);
         EnableWindow(GetDlgItem(hDlg, IDC_SETUP_DESK), FALSE);
         EnableWindow(GetDlgItem(hDlg, IDC_REMOVE_DESK), FALSE);

         //Setup the column number controls
         hWnd = GetDlgItem(hDlg, IDC_COLUMN_NUMBER);
         SendMessage(hWnd, EM_LIMITTEXT, (WPARAM)2, (LPARAM)0);
         SetDlgItemInt(hDlg, IDC_COLUMN_NUMBER, deskMan->GetNbColumns(), FALSE);

         hWnd = GetDlgItem(hDlg, IDC_COLUMN_SPIN);
         SendMessage(hWnd, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)99, (short) 1));
         SendMessage(hWnd, UDM_SETPOS, 0, (LPARAM) MAKELONG((short) deskMan->GetNbColumns(), 1));
      }
		return TRUE;

	case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case IDC_INSERT_DESK:
         {
            HWND listBox = GetDlgItem(hDlg, IDC_DESK_LIST);
            Desktop * desk = deskMan->AddDesktop();
            LRESULT index = SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)desk->GetText());
            SendMessage(listBox, LB_SETITEMDATA, index, (LPARAM)desk);
            bool result = (SendMessage(listBox, LB_SETCURSEL, index, 0) != LB_ERR);
            EnableWindow(GetDlgItem(hDlg, IDC_REMOVE_DESK), result);
            EnableWindow(GetDlgItem(hDlg, IDC_SETUP_DESK), result);
            EnableWindow(GetDlgItem(hDlg, IDC_DESK_SPIN), result);
            vdWindow.Refresh();
         }
         break;

      case IDC_REMOVE_DESK:
         {
            HWND listBox = GetDlgItem(hDlg, IDC_DESK_LIST);
            LRESULT index;

            //To not delete the last desktop, as the following code would not display the content of the
            //list properly in that case (due to the desktop being automatically created)
            if (deskMan->GetNbDesktops() == 1)
            {
               locMessageBox(hDlg, IDS_NEEDADESKTOP_ERROR, IDS_UNABLETOREMOVEDESKTOP, MB_OK|MB_ICONERROR);
               break;
            }

            index = SendMessage(listBox, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR)
            {
               deskMan->RemoveDesktop((Desktop*)SendMessage(listBox, LB_GETITEMDATA, index, 0));
               SendMessage(listBox, LB_DELETESTRING, index, 0);
               if ( (SendMessage(listBox, LB_SETCURSEL, index, 0) == LB_ERR) &&  //Try to activate the next one
                    (SendMessage(listBox, LB_SETCURSEL, index-1, 0) == LB_ERR) ) //Else, activate the previous one
               {
                  EnableWindow(GetDlgItem(hDlg, IDC_REMOVE_DESK), FALSE);
                  EnableWindow(GetDlgItem(hDlg, IDC_SETUP_DESK), FALSE);
               }
               else
                  EnableWindow(GetDlgItem(hDlg, IDC_REMOVE_DESK), deskMan->GetNbDesktops()>1 ? TRUE : FALSE);
               vdWindow.Refresh();
            }
            else
               MessageBeep(MB_ICONEXCLAMATION);
         }
         break;

      case IDC_SETUP_DESK:
         {
            HWND listBox = GetDlgItem(hDlg, IDC_DESK_LIST);
            LRESULT index;
            index = SendMessage(listBox, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR)
            {
               Desktop * desk = (Desktop*)SendMessage(listBox, LB_GETITEMDATA, index, 0);
               if (desk->Configure(hDlg))
               {
                  /* Update the list box content */
                  SendMessage(listBox, LB_DELETESTRING, index, 0);
                  index = SendMessage(listBox, LB_INSERTSTRING, index, (LPARAM)desk->GetText());
                  SendMessage(listBox, LB_SETITEMDATA, index, (LPARAM)desk);
                  SendMessage(listBox, LB_SETCURSEL, index, 0);

                  /* Refresh the main window */
                  vdWindow.Refresh();
               }
            }
         }
         break;

      case IDC_DESK_LIST:
         switch(HIWORD(wParam))
         {
         case LBN_SELCANCEL:
            EnableWindow(GetDlgItem(hDlg, IDC_DESK_SPIN), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_REMOVE_DESK), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_SETUP_DESK), FALSE);
            break;

         case LBN_SELCHANGE:
            {
               HWND hWnd;

               hWnd = GetDlgItem(hDlg, IDC_DESK_LIST);
               short pos = (short)SendMessage(hWnd, LB_GETCURSEL, 0, 0);
               short max = (short)SendMessage(hWnd, LB_GETCOUNT, 0, 0)-1;

               hWnd = GetDlgItem(hDlg, IDC_DESK_SPIN);
               EnableWindow(hWnd, TRUE);
               SendMessage(hWnd, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short) max, (short) 0));
               SendMessage(hWnd, UDM_SETPOS, 0, (LPARAM) MAKELONG((short) pos, 0));

               EnableWindow(GetDlgItem(hDlg, IDC_REMOVE_DESK), deskMan->GetNbDesktops()>1 ? TRUE : FALSE);
               EnableWindow(GetDlgItem(hDlg, IDC_SETUP_DESK), TRUE);
            }
            break;

         case LBN_DBLCLK:
            PostMessage(hDlg, WM_COMMAND, IDC_SETUP_DESK, 0);
            break;
         }
         break;

      case IDC_COLUMN_NUMBER:
         switch(HIWORD(wParam))
         {
         case EN_CHANGE:
            {
               int nbCols=0;

               if (!IsWindowVisible(hDlg))
                  break;

               /* Get the current value */
               nbCols = GetDlgItemInt(hDlg, IDC_COLUMN_NUMBER, NULL, FALSE);

               /* Ensure it is greater than 0 */
               if (nbCols < 1)
               {
                  nbCols = 1;
                  SetDlgItemInt(hDlg, IDC_COLUMN_NUMBER, 1, FALSE);
               }

               /* Save the change */
               deskMan->SetNbColumns(nbCols);

               /* Update the window */
               vdWindow.Refresh();
            }
            break;
         }
         break;
      }
      break;

   case WM_NOTIFY:
      LPNMHDR pnmh = (LPNMHDR) lParam;
      assert(wParam == pnmh->idFrom);
      switch(pnmh->idFrom)
      {
      case IDC_DESK_SPIN:
         if (pnmh->code == UDN_DELTAPOS)
         {
            LPNM_UPDOWN lpnmud = (LPNM_UPDOWN) lParam;
            HWND listBox = GetDlgItem(hDlg, IDC_DESK_LIST);
            LRESULT index = SendMessage(listBox, LB_GETCURSEL, 0, 0);

            if (index != LB_ERR)
            {
               Desktop * desk = (Desktop*)SendMessage(listBox, LB_GETITEMDATA, index, 0);
               LRESULT newIndex;
               LRESULT nbDesks = SendMessage(listBox, LB_GETCOUNT, 0, 0);

               //Reorder the items in the list box
               newIndex = index - lpnmud->iDelta;
               if ((newIndex < 0) || (newIndex >= nbDesks))
                  break;
               SendMessage(listBox, LB_DELETESTRING, index, 0);
               newIndex = SendMessage(listBox, LB_INSERTSTRING, newIndex/*+1*/, (LPARAM)desk->GetText());
               SendMessage(listBox, LB_SETITEMDATA, newIndex, (LPARAM)desk);
               SendMessage(listBox, LB_SETCURSEL, newIndex, 0);

               //Set the desks index accordingly, and sort the desks according to their indexes
               for(int i=0; i<nbDesks; i++)
               {
                  Desktop * desk = (Desktop*)SendMessage(listBox, LB_GETITEMDATA, i, 0);
                  desk->SetIndex(i);
               }
               deskMan->Sort();

               deskMan->UpdateLayout();

               //Refresh the main window
               vdWindow.Refresh();
            }
            else
               MessageBeep(MB_ICONEXCLAMATION);
         }
         break;

      default:
         switch (pnmh->code)
         {
         case PSN_KILLACTIVE:
            SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, FALSE);
            return TRUE;

         case PSN_APPLY:
            SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
         }
      }
      break;
	}
	return FALSE;
}

// Message handler for the OSD settings page.
LRESULT CALLBACK OSDConfiguration(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
      {
         HWND hWnd;
         bool supportTransparency;   //is transparency supported on the platform ?
         OnScreenDisplayWnd * osd = deskMan->GetOSDWindow();
         bool osdEnabled;

         supportTransparency = Transparency::IsTransparencySupported();
         osdEnabled = deskMan->IsOSDEnabled();

         //Setup display OSD
         hWnd = GetDlgItem(hDlg, IDC_ENABLEOSD_CHECK);
         SendMessage(hWnd, BM_SETCHECK, osdEnabled ? BST_CHECKED : BST_UNCHECKED, 0);

         //Setup the transparency slider and associated controls
         hWnd = GetDlgItem(hDlg, IDC_TRANSP_SLIDER);
         SendMessage(hWnd, TBM_SETRANGE, FALSE, MAKELONG(0,255));
         SendMessage(hWnd, TBM_SETTICFREQ, 16, 0);
         SendMessage(hWnd, TBM_SETBUDDY, TRUE, (LPARAM)GetDlgItem(hDlg, IDC_TRANSP_STATIC1));
         SendMessage(hWnd, TBM_SETBUDDY, FALSE, (LPARAM)GetDlgItem(hDlg, IDC_TRANSP_STATIC2));
         SendMessage(hWnd, TBM_SETPOS, TRUE, osd->GetTransparencyLevel());
         EnableWindow(hWnd, supportTransparency && osdEnabled);

         EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC), supportTransparency && osdEnabled);
         EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC1), supportTransparency && osdEnabled);
         EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC2), supportTransparency && osdEnabled);

         //Setup disable mouse input
         hWnd = GetDlgItem(hDlg, IDC_TRANSPARENT_CHK);
         SendMessage(hWnd, BM_SETCHECK, osd->IsTransparent() ? BST_CHECKED : BST_UNCHECKED, 0);
         EnableWindow(hWnd, osdEnabled);

         //Setup shade background
         hWnd = GetDlgItem(hDlg, IDC_BACKGROUND_CHK);
         SendMessage(hWnd, BM_SETCHECK, osd->HasBackground() ? BST_CHECKED : BST_UNCHECKED, 0);
         EnableWindow(hWnd, osdEnabled);

         //Setup timeout controls
         SetDlgItemInt(hDlg, IDC_TIMEOUT_EDIT, osd->GetDefaultTimeout(), FALSE);
         EnableWindow(hWnd, osdEnabled);

         hWnd = GetDlgItem(hDlg, IDC_TIMEOUT_SPIN);
         SendMessage(hWnd, UDM_SETRANGE, 0, MAKELPARAM(UD_MAXVAL, 0));
         SendMessage(hWnd, UDM_SETPOS, 0, MAKELPARAM((short)osd->GetDefaultTimeout(), 0));
         EnableWindow(hWnd, osdEnabled);

         EnableWindow(GetDlgItem(hDlg, IDC_TIMEOUT_STATIC), osdEnabled);

         //Setup font button
         hWnd = GetDlgItem(hDlg, IDC_FONT_BTN);
         EnableWindow(hWnd, osdEnabled);

         //Setup background color button
         hWnd = GetDlgItem(hDlg, IDC_BGCOLOR_BTN);
         EnableWindow(hWnd, osdEnabled);

         //Setup position button
         hWnd = GetDlgItem(hDlg, IDC_POSITION_BTN);
         EnableWindow(hWnd, false && osdEnabled);
      }
		return TRUE;

   case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case IDC_FONT_BTN:
         deskMan->GetOSDWindow()->SelectFont();
         break;

      case IDC_BGCOLOR_BTN:
         deskMan->GetOSDWindow()->SelectBgColor();
         break;

      case IDC_POSITION_BTN:
         break;

      case IDC_ENABLEOSD_CHECK:
         {
            bool supportTransparency = Transparency::IsTransparencySupported();
            bool osdEnabled = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;

            EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_SLIDER), supportTransparency && osdEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC), supportTransparency && osdEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC1), supportTransparency && osdEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC2), supportTransparency && osdEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_TRANSPARENT_CHK), osdEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_BACKGROUND_CHK), osdEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_TIMEOUT_EDIT), osdEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_TIMEOUT_SPIN), osdEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_TIMEOUT_STATIC), osdEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_FONT_BTN), osdEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_BGCOLOR_BTN), osdEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_POSITION_BTN), false && osdEnabled);
         }
         break;
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
         {
            HWND hWnd;
            OnScreenDisplayWnd * osd = deskMan->GetOSDWindow();

            //Apply enable OSD
            hWnd = GetDlgItem(hDlg, IDC_ENABLEOSD_CHECK);
            deskMan->EnableOSD(SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);

            //Apply the transparency slider and associated controls
            hWnd = GetDlgItem(hDlg, IDC_TRANSP_SLIDER);
            osd->SetTransparencyLevel((unsigned char)SendMessage(hWnd, TBM_GETPOS, 0, 0));

            //Apply disable mouse input
            hWnd = GetDlgItem(hDlg, IDC_TRANSPARENT_CHK);
            osd->MakeTransparent(SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);

            //Apply shade background
            hWnd = GetDlgItem(hDlg, IDC_BACKGROUND_CHK);
            osd->EnableBackground(SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ?  true : false);

            //Apply timeout
            osd->SetDefaultTimeout(GetDlgItemInt(hDlg, IDC_TIMEOUT_EDIT, NULL, FALSE));

            //Apply succeeded
            SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
         }
         return TRUE;
      }
      break;
	}
	return FALSE;
}

// Message handler for the troubleshooting settings page.
LRESULT CALLBACK TroubleShootingConfiguration(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_INITDIALOG:
      {
         Settings settings;
         int hidingmethod;

         //Setup integrate with shell
         CheckDlgButton(hDlg, IDC_INTEGRATEWTHSHELL_CHECK, winMan->IsIntegrateWithShell() ? BST_CHECKED : BST_UNCHECKED);

         //Setup hiding method
         hidingmethod = settings.LoadSetting(Settings::DefaultHidingMethod);
         if (hidingmethod > 2 || hidingmethod < 0)
            hidingmethod = 0;
         CheckRadioButton(hDlg, IDC_HIDEMETHOD_RADIO, IDC_MOVEMETHOD_RATIO, IDC_HIDEMETHOD_RADIO+hidingmethod);
      }
      return TRUE;

   case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case IDC_HIDINGMETHODEXCEPTIONS_BTN:
         {
				LPCTSTR values[4];
				char * coltext;
            Settings settings;
				locGetString(values[0], IDS_METHOD_HIDE);
				locGetString(values[1], IDS_METHOD_MINIMIZE);
				locGetString(values[2], IDS_METHOD_MOVE);
				values[3] = NULL;
				locGetString(coltext, IDS_METHOD);
            Config::Group * group = settings.GetHidingMethodExceptions();
            ApplicationListDlg dlg(group, coltext, settings.LoadSetting(Settings::DefaultHidingMethod), values);
				dlg.ShowDialog(Locale::GetInstance(), hDlg);
            delete group;
         }
         break;

      case IDC_SHELLINTEGEXCEPTION_BTN:
         {
            Settings settings;
            Config::Group * group = settings.GetShellIntegrationExceptions();
            ApplicationListDlg dlg(group);
            dlg.ShowDialog(Locale::GetInstance(), hDlg);
            delete group;
         }
         break;
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
         {
            int i;

            winMan->SetIntegrateWithShell(IsDlgButtonChecked(hDlg, IDC_INTEGRATEWTHSHELL_CHECK) == BST_CHECKED);

            for(i = IDC_HIDEMETHOD_RADIO; i <= IDC_MOVEMETHOD_RATIO; i++)
               if (IsDlgButtonChecked(hDlg, i))
               {
                  Settings settings;
                  settings.SaveSetting(Settings::DefaultHidingMethod, i-IDC_HIDEMETHOD_RADIO);
                  break;
               }

            //Apply succeeded
            SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
         }
         return TRUE;
      }
      break;
   }
   return FALSE;
}

//Property Sheet initialization
HWND CreateConfigBox()
{
   PROPSHEETPAGE pages[6];
   PROPSHEETHEADER propsheet;
	HINSTANCE hresinst = Locale::GetInstance();

   memset(&pages, 0, sizeof(pages));

   pages[0].dwSize = sizeof(PROPSHEETPAGE);
   pages[0].hInstance = hresinst;
   pages[0].dwFlags = PSP_DEFAULT;
   pages[0].pfnDlgProc = (DLGPROC)SettingsConfiguration;
   pages[0].pszTemplate = MAKEINTRESOURCE(IDD_GLOBAL_SETTINGS);

   pages[1].dwSize = sizeof(PROPSHEETPAGE);
   pages[1].hInstance = hresinst;
   pages[1].dwFlags = PSP_DEFAULT;
   pages[1].pfnDlgProc = (DLGPROC)DisplayConfiguration;
   pages[1].pszTemplate = MAKEINTRESOURCE(IDD_DISPLAY_SETTINGS);

   pages[2].dwSize = sizeof(PROPSHEETPAGE);
   pages[2].hInstance = hresinst;
   pages[2].dwFlags = PSP_DEFAULT;
   pages[2].pfnDlgProc = (DLGPROC)DeskConfiguration;
   pages[2].pszTemplate = MAKEINTRESOURCE(IDD_DESKS_SETTINGS);

   pages[3].dwSize = sizeof(PROPSHEETPAGE);
   pages[3].hInstance = hresinst;
   pages[3].dwFlags = PSP_DEFAULT;
   pages[3].pfnDlgProc = (DLGPROC)OSDConfiguration;
   pages[3].pszTemplate = MAKEINTRESOURCE(IDD_OSD_SETTINGS);

   pages[4].dwSize = sizeof(PROPSHEETPAGE);
   pages[4].hInstance = hresinst;
   pages[4].dwFlags = PSP_DEFAULT;
   pages[4].pfnDlgProc = ShortcutsConfigurationDlg::GetWindowProc();
   pages[4].pszTemplate = MAKEINTRESOURCE(IDD_SHORTCUT_SETTINGS);

   pages[5].dwSize = sizeof(PROPSHEETPAGE);
   pages[5].hInstance = hresinst;
   pages[5].dwFlags = PSP_DEFAULT;
   pages[5].pfnDlgProc = (DLGPROC)TroubleShootingConfiguration;
   pages[5].pszTemplate = MAKEINTRESOURCE(IDD_TROUBLESHOOTING_SETTINGS);

   memset(&propsheet, 0, sizeof(propsheet));
   propsheet.dwSize = sizeof(PROPSHEETHEADER);
   propsheet.dwFlags = PSH_MODELESS | PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
   propsheet.hwndParent = vdWindow;
   propsheet.hInstance = hresinst;
   propsheet.pszCaption = MAKEINTRESOURCE(IDS_DLGNAME_SETTINGS);
   propsheet.nPages = sizeof(pages)/sizeof(PROPSHEETPAGE);
   propsheet.ppsp = pages;

   return (HWND)PropertySheet(&propsheet);
}
