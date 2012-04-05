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
#include "movewindow.h"
#include "VirtualDimension.h"
#include "DesktopManager.h"
#include "Locale.h"

Window * movedWindow;

void ApplySettings(HWND hDlg)
{
   HWND hWnd;
   int curSel;
   Desktop * desk;

   //Disable the APPLY button
   EnableWindow(GetDlgItem(hDlg, IDC_APPLY), FALSE);

   if (IsDlgButtonChecked(hDlg, IDC_ALLDESKS_CHECK))
      desk = NULL;
   else
   {
      //Get the current selection
      hWnd = GetDlgItem(hDlg, IDC_DESK_LIST);
      curSel = SendMessage(hWnd, LB_GETCURSEL, 0, 0);
      if (curSel == LB_ERR)
         return;

      //Get a pointer to the desktop object associated with the selection
      desk = (Desktop *)SendMessage(hWnd, LB_GETITEMDATA, curSel, 0);
      if (desk == NULL)
         return;
   }

   //Move the window to the appropriate desktop
   movedWindow->MoveToDesktop(desk);
   vdWindow.Refresh();
}

LRESULT CALLBACK MoveWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
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

         //Select the desktops on which the window is
         if (movedWindow->IsOnDesk(NULL))
         {
            CheckDlgButton(hDlg, IDC_ALLDESKS_CHECK, TRUE);
            SetFocus(GetDlgItem(hDlg, IDC_ALLDESKS_CHECK));
         }
         else
         {
            for(int index = 0;
               index < SendMessage(hWnd, LB_GETCOUNT, 0, 0);
               index ++)
            {
               Desktop * desk;
               
               desk = (Desktop *)SendMessage(hWnd, LB_GETITEMDATA, index, 0);
               if (movedWindow->IsOnDesk(desk))
                  SendMessage(hWnd, LB_SETCURSEL, index, 0);
            }
            SetFocus(hWnd);
         }

         //Disable the APPLY button
         EnableWindow(GetDlgItem(hDlg, IDC_APPLY), FALSE);
      }
		return FALSE;

	case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case IDC_APPLY:
         ApplySettings(hDlg);
         break;

      case IDOK:
         ApplySettings(hDlg);
         /* do not break, to execute the closing code (IDCANCEL statement) */
         
      case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;

      case IDC_DESK_LIST:
         switch (HIWORD(wParam))
         {
         case LBN_DBLCLK:
            PostMessage(hDlg, WM_COMMAND, IDOK, 0);
            break;

         case LBN_SELCHANGE:
            CheckDlgButton(hDlg, IDC_ALLDESKS_CHECK, FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_APPLY), TRUE);
            break;
         }
         break;

      case IDC_ALLDESKS_CHECK:
         SendMessage(GetDlgItem(hDlg, IDC_DESK_LIST), LB_SETCURSEL, (WPARAM)-1, 0);
         EnableWindow(GetDlgItem(hDlg, IDC_APPLY), TRUE);
         break;
      }
      break;
   }

   return FALSE;
}

void SelectDesktopForWindow(Window * window)
{
   movedWindow = window;
   
	DialogBox(Locale::GetInstance(), MAKEINTRESOURCE(IDD_MOVEWINDOW), vdWindow, (DLGPROC)MoveWindowProc);
}
