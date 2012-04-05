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
#include <WindowsX.h>
#include <CommDlg.h>
#include <ShellAPI.h>
#include "Resource.h"
#include "ApplicationListDlg.h"
#include "PlatformHelper.h"
#include "Locale.h"

ApplicationListDlg::ApplicationListDlg(Config::Group * group, LPCTSTR title, int defaultValue, const LPCTSTR * values)
{
   m_appgroup = group;
   m_defaultValue = defaultValue;
   m_values = values;
   m_valTitle = title;
   m_hParamEditCtrl = NULL;
}

ApplicationListDlg::~ApplicationListDlg(void)
{
   DestroyWindow(m_hParamEditCtrl);
}

int ApplicationListDlg::ShowDialog(HINSTANCE hinstance, HWND hWndParent)
{
   return ::DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_APPLLIST_DLG), hWndParent, &DlgProc, (LPARAM)this);
}

void ApplicationListDlg::InitDialog()
{
   LVCOLUMN column;

   //Create the image list
   //Note: no need to destroy this list, as it will be destroyed automatically 
   //when the list it is associated with is destroyed.
   m_hImgList = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK, 10, 5);
   m_defaultIconIdx = ImageList_AddIcon(m_hImgList, (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DEFAPP_SMALL), IMAGE_ICON, 16, 16, LR_SHARED));

   //Setup the program list
   m_hAppListWnd = GetDlgItem(m_hDlg, IDC_APPL_LIST);

   ListView_SetExtendedListViewStyleEx(m_hAppListWnd, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
   ListView_SetImageList(m_hAppListWnd, m_hImgList, LVSIL_SMALL);

   column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
   column.fmt = LVCFMT_LEFT|LVCFMT_IMAGE;
   column.cx = m_values ? 250 : 320;
   locGetString(column.pszText, IDS_PROGRAM);
   column.iSubItem = -1;
   ListView_InsertColumn(m_hAppListWnd, 0, &column);

   if (m_valTitle)
   {
      column.fmt = LVCFMT_LEFT;
      column.cx = 70;
      column.pszText = (LPTSTR)m_valTitle;
      column.iSubItem = 0;
      ListView_InsertColumn(m_hAppListWnd, 1, &column);

      if (m_values)
      {
         m_hParamEditCtrl = CreateWindow("COMBOBOX", "", WS_BORDER|WS_CHILD|CBS_DROPDOWNLIST, 0, 0, 10, 10, m_hAppListWnd, NULL, GetModuleHandle(NULL), 0);

         const LPCTSTR * val = m_values;
         while(*val)
            ComboBox_AddString(m_hParamEditCtrl, *val++);
      }
      else
         m_hParamEditCtrl = CreateWindow("EDIT", "", WS_BORDER|WS_CHILD|ES_NUMBER|ES_AUTOHSCROLL, 0, 0, 0, 0, m_hAppListWnd, NULL, GetModuleHandle(NULL), 0);

      SetWindowFont(m_hParamEditCtrl, GetWindowFont(m_hDlg), FALSE);
   }

   //Populate program list
   int i;
   TCHAR filename[MAX_PATH];
   DWORD length = MAX_PATH;
   for(i=0; length=MAX_PATH, m_appgroup->EnumEntry(i, filename, &length); i++)
      InsertProgram(filename, m_appgroup->LoadDWord(filename, m_defaultValue));
}

void ApplicationListDlg::OnInsertApplBtn()
{
   TCHAR path[MAX_PATH];

   //Stop editing item
   if (IsEditing())
      EndEdit();

   //No file at the moment
   *path = 0;

   //Browse for a new program name
   if (GetProgramName(path, MAX_PATH) && 
       (FindProgram(path)==-1 || (locMessageBox(m_hDlg, IDS_PROGRAMINLIST_ERROR, IDS_ERROR, MB_OK|MB_ICONEXCLAMATION), FALSE)))
      InsertProgram(path, m_defaultValue);
}

void ApplicationListDlg::OnEditApplBtn()
{
   TCHAR path[MAX_PATH];
   int idx;
	int newidx;

   //Stop editing item
   if (IsEditing())
      EndEdit();

   //Get selected item index
   idx = ListView_GetNextItem(m_hAppListWnd, (WPARAM)-1, LVNI_SELECTED);
   if (idx == -1)
      return;

   //Get current program name of the item
   ListView_GetItemText(m_hAppListWnd, idx, 0, path, MAX_PATH);

   //Browse for a new program name
   if (GetProgramName(path, MAX_PATH) && 
		 (newidx = FindProgram(path), TRUE) &&
       (newidx==-1 || newidx==idx || (locMessageBox(m_hDlg, IDS_PROGRAMINLIST_ERROR, IDS_ERROR, MB_OK|MB_ICONEXCLAMATION), FALSE)))
      InsertProgram(path, m_defaultValue, idx);
}

void ApplicationListDlg::OnRemoveApplBtn()
{
   //Stop editing item
   if (IsEditing())
      EndEdit();

   //Get selected item index
   int idx = ListView_GetNextItem(m_hAppListWnd, (WPARAM)-1, LVNI_SELECTED);

   //Remove it from the list
   if (idx != -1)
      ListView_DeleteItem(m_hAppListWnd, idx);
}

void ApplicationListDlg::BeginEdit(int item)
{
   char buf[6];
   DWORD param = GetProgramParam(item);

   m_editedItemIndex = item;

   //Set the param value
   if (m_values)
      ComboBox_SetCurSel(m_hParamEditCtrl, param);
   else
      SetWindowText(m_hParamEditCtrl, itoa(param, buf, 10));

   //Display the window at the correct location
   RECT rect;
   ListView_GetSubItemRect(m_hAppListWnd, item, 1, LVIR_BOUNDS, &rect);
   MoveWindow(m_hParamEditCtrl, rect.left, rect.top-1, rect.right-rect.left+1, rect.bottom-rect.top+1, TRUE);
   ShowWindow(m_hParamEditCtrl, SW_SHOW);

   //Give the focus to the edit control
   SetFocus(m_hParamEditCtrl);
}

void ApplicationListDlg::EndEdit()
{
   DWORD param;
   char buf[6];

   if (m_values)
   {
      param = ComboBox_GetCurSel(m_hParamEditCtrl);
   }
   else
   {
      GetWindowText(m_hParamEditCtrl, buf, 6);
      param = atoi(buf);
   }

   //Update the list
   SetProgramParam(m_editedItemIndex, param);

   //Hide the edit control
   ShowWindow(m_hParamEditCtrl, SW_HIDE);
}

void ApplicationListDlg::OnProgramClick(LPNMITEMACTIVATE lpnmitem)
{
   if (IsEditing())
      EndEdit();

   if (lpnmitem->iItem != -1 && lpnmitem->iSubItem == 1)
      BeginEdit(lpnmitem->iItem);
}

void ApplicationListDlg::OnProgramSetFocus()
{
   if (IsEditing())
      EndEdit();
}

void ApplicationListDlg::OnApply()
{
   int i = 0;
   TCHAR filename[MAX_PATH];
   DWORD length = MAX_PATH;

   //Validate current parameter, if any
   if (IsEditing())
      EndEdit();

   //Remove entries which have been removed (ie are not in the list box)
   while(length=MAX_PATH, m_appgroup->EnumEntry(i, filename, &length))
   {
      if (FindProgram(filename) != -1)
         i++;
      else
         m_appgroup->RemoveEntry(filename);
   }

   //Add/set value for all entries in the list box
   for(i=0; i<ListView_GetItemCount(m_hAppListWnd); i++)
   {
      ListView_GetItemText(m_hAppListWnd, i, 0, filename, length);
      m_appgroup->SaveDWord(filename, GetProgramParam(i));
   }
}

BOOL ApplicationListDlg::GetProgramName(LPTSTR filename, DWORD maxlen)
{
   OPENFILENAME ofn;

	String filter;

   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = m_hDlg;
   ofn.lpstrFile = filename;
   ofn.nMaxFile = maxlen;
	filter = Locale::GetInstance().GetString(IDS_PROGRAMFILTER);
	filter.Replace('|', 0);	
   ofn.lpstrFilter = filter;
   ofn.nFilterIndex = 1;
   ofn.lpstrFileTitle = NULL;
   ofn.nMaxFileTitle = 0;
   ofn.lpstrInitialDir = NULL;
	locGetString(ofn.lpstrTitle, IDS_ADDPROGRAM);
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER /*| OFN_ENABLESIZING*/;

   return GetOpenFileName(&ofn);
}

int ApplicationListDlg::FindProgram(LPTSTR filename)
{
   LVFINDINFO fi;

   fi.flags = LVFI_STRING;
   fi.psz = filename;

   return ListView_FindItem(m_hAppListWnd, -1, &fi);
}

void ApplicationListDlg::InsertProgram(LPTSTR filename, int value, int idx)
{
   LVITEM item;
   HICON appicon;

   //Load the icon for this application
   if (ExtractIconEx(filename, 0, NULL, &appicon, 1))
   {
      item.iImage = ImageList_AddIcon(m_hImgList, appicon);
      DestroyIcon(appicon);
   }
   else
      item.iImage = m_defaultIconIdx;

   //Insert the new item
   item.mask = LVIF_TEXT | LVIF_IMAGE;
   item.pszText = filename;
   item.iItem = idx == -1 ? ListView_GetItemCount(m_hAppListWnd) : idx;
   item.iSubItem = 0;
   if (idx == -1)
      idx = ListView_InsertItem(m_hAppListWnd, &item);
   else
      ListView_SetItem(m_hAppListWnd, &item);

   //Set the value
   SetProgramParam(idx, value);
}

void ApplicationListDlg::SetProgramParam(int item, DWORD param)
{
   LVITEM lvitem;

   //Store the value
   lvitem.mask = LVIF_PARAM;
   lvitem.iItem = item;
   lvitem.iSubItem = 0;
   lvitem.lParam = param;
   ListView_SetItem(m_hAppListWnd, &lvitem);

   //Setup sub-item in the list
   if (m_values)
   {
      ListView_SetItemText(m_hAppListWnd, item, 1, (LPTSTR)m_values[param]);
   }
   else if (m_valTitle) //else, this is not needed, as the data will not be shown/changed anyway
   {
      char buf[6];
      itoa(param, buf, 10);
      ListView_SetItemText(m_hAppListWnd, item, 1, buf);
   }
}

DWORD ApplicationListDlg::GetProgramParam(int item)
{
   LVITEM lvitem;

   lvitem.mask = LVIF_PARAM;
   lvitem.iItem = item;
   lvitem.iSubItem = 0;
   ListView_GetItem(m_hAppListWnd, &lvitem);

   return lvitem.lParam;
}

INT_PTR CALLBACK ApplicationListDlg::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   ApplicationListDlg * self;

   switch (message)
   {
   case WM_INITDIALOG:
      self = (ApplicationListDlg*)lParam;
      self->m_hDlg = hDlg;
      SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)self);
      self->InitDialog();
      return TRUE;

   case WM_COMMAND:
      self = (ApplicationListDlg *)GetWindowLongPtr(hDlg, DWLP_USER);
      switch(LOWORD(wParam))
      {
      case IDOK:
         self->OnApply();
      case IDCANCEL:
         EndDialog(hDlg, LOWORD(wParam));
         break;

      case IDC_INSERTAPPL_BTN:
         self->OnInsertApplBtn();
         break;

      case IDC_EDITAPPL_BTN:
         self->OnEditApplBtn();
         break;

      case IDC_REMOVEAPPL_BTN:
         self->OnRemoveApplBtn();
         break;
      }
      break;

   case WM_NOTIFY:
      LPNMHDR pnmh = (LPNMHDR) lParam;
      self = (ApplicationListDlg *)GetWindowLongPtr(hDlg, DWLP_USER);
      switch (pnmh->code)
      {
      case NM_CLICK:
         if (GetDlgCtrlID(pnmh->hwndFrom) == IDC_APPL_LIST)
            self->OnProgramClick((LPNMITEMACTIVATE) lParam);
         break;

      case NM_SETFOCUS:
         if (GetDlgCtrlID(pnmh->hwndFrom) == IDC_APPL_LIST)
            self->OnProgramSetFocus();
         break;
      }
      break;
   }

   return FALSE;
}
