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
#include "HotkeyConfig.h"
#include "VirtualDimension.h"
#include "Resource.h"
#include "Locale.h"

list<ConfigurableHotkey*> ShortcutsConfigurationDlg::m_hotkeys;

ConfigurableHotkey::ConfigurableHotkey(): m_hotkey(0)  
{
   ShortcutsConfigurationDlg::RegisterHotkey(this);
}

ConfigurableHotkey::~ConfigurableHotkey()      
{
   ShortcutsConfigurationDlg::UnRegisterHotkey(this); 
   if (m_hotkey) 
      HotKeyManager::GetInstance()->UnregisterHotkey(this);
}

int ConfigurableHotkey::GetHotkey() const          
{
   return m_hotkey; 
}
      
bool ConfigurableHotkey::SetHotkey(int hotkey)
{
   HotKeyManager * keyMan = HotKeyManager::GetInstance();

   //If we are not changing the hotkey, nothing to do
   if (hotkey == m_hotkey)
      return false;

   //Unregister the previous hotkey
   if (m_hotkey != 0)
      keyMan->UnregisterHotkey(this);
   
   m_hotkey = hotkey;

   //Setting the hotkey to 0 removes the shortcut
   if (m_hotkey == 0)
      return true;

   //Register the new hotkey
   keyMan->RegisterHotkey(m_hotkey, this);

	return true;
}

extern void GetShortcutName(int shortcut, char* str, int bufLen);

void ShortcutsConfigurationDlg::InsertItem(ConfigurableHotkey* hotkey)
{
   LVITEM item;

   item.pszText = (LPSTR)hotkey->GetName();
   item.iItem = ListView_GetItemCount(m_listViewWnd);
   item.iSubItem = 0;
   item.lParam = (LPARAM)hotkey;
   item.mask = LVIF_TEXT | LVIF_PARAM;
   int index = ListView_InsertItem(m_listViewWnd, &item);

   SetItemShortcut(index, hotkey->GetHotkey());
}

ConfigurableHotkey* ShortcutsConfigurationDlg::GetItemHotkey(int index)
{
   LVITEM lvItem;
   lvItem.mask = LVIF_PARAM;
   lvItem.iItem = index;
   lvItem.iSubItem = 0;
   ListView_GetItem(m_listViewWnd, &lvItem);
   return (ConfigurableHotkey*)lvItem.lParam;
}

void ShortcutsConfigurationDlg::SetItemShortcut(int index, int shortcut)
{
   char buffer[50];
   GetShortcutName(shortcut, buffer, sizeof(buffer)/sizeof(char));
   ListView_SetItemText(m_listViewWnd, index, 1, buffer);

   GetItemHotkey(index)->m_tempHotkey = shortcut;
}

int ShortcutsConfigurationDlg::GetItemShortcut(int index)
{
   return GetItemHotkey(index)->m_tempHotkey;
}

void ShortcutsConfigurationDlg::BeginEdit(int item)
{
   m_editedItemIndex = item;

   //Set the hotkey
   SendMessage(m_editCtrl, HKM_SETHOTKEY, GetItemShortcut(item), 0);

   //Display the window at the correct location
   RECT rect;
   ListView_GetSubItemRect(m_listViewWnd, item, 1, LVIR_BOUNDS, &rect);
   MoveWindow(m_editCtrl, rect.left, rect.top-1, rect.right-rect.left+1, rect.bottom-rect.top+1, TRUE);
   ShowWindow(m_editCtrl, SW_SHOW);

   //Give the focus to the edit control
   SetFocus(m_editCtrl);
}

void ShortcutsConfigurationDlg::EndEdit()
{
   int key = (LPARAM)SendMessage(m_editCtrl, HKM_GETHOTKEY, 0, 0);

   //Update the list
   SetItemShortcut(m_editedItemIndex, key);

   //Hide the edit control
   ShowWindow(m_editCtrl, SW_HIDE);
}

ShortcutsConfigurationDlg::ShortcutsConfigurationDlg(HWND hDlg)
{
   LVCOLUMN column;
	String text;

   m_hDlg = hDlg;

   //Setup list view
   m_listViewWnd = GetDlgItem(hDlg, IDC_SHORTCUTSLIST);
   ListView_SetExtendedListViewStyleEx(m_listViewWnd, 
                                       LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

   column.mask = LVCF_TEXT;
	text = Locale::GetInstance().GetString(IDS_COLUMN_FUNCTION);
   column.pszText = text.GetBuffer();
   ListView_InsertColumn(m_listViewWnd, 1, &column);
	text.ReleaseBuffer();

   column.mask = LVCF_SUBITEM | LVCF_FMT | LVCF_TEXT;
	text = Locale::GetInstance().GetString(IDS_COLUMN_SHORTCUT);
	column.pszText = text.GetBuffer();
   column.iSubItem = 0;
   column.fmt = LVCFMT_LEFT;
   ListView_InsertColumn(m_listViewWnd, 1, &column);
	text.ReleaseBuffer();

   //Add all registered shortcuts
   for(list<ConfigurableHotkey*>::iterator it = m_hotkeys.begin(); it != m_hotkeys.end(); it++)
      InsertItem(*it);
   
   //Resize all columns to fit
   for(int i=0; i<2; i++)
      ListView_SetColumnWidth(m_listViewWnd, i, -2);

   //Create the edit control
   m_editCtrl = CreateWindow("AlternateHotKeyControl", "", WS_BORDER | WS_CHILD | WS_TABSTOP, 0, 0, 10, 10, 
                             m_listViewWnd, NULL, vdWindow, 0);
}

ShortcutsConfigurationDlg::~ShortcutsConfigurationDlg()
{
   DestroyWindow(m_editCtrl);
   ListView_DeleteAllItems(m_listViewWnd);
   ListView_DeleteColumn(m_listViewWnd, 0);
   ListView_DeleteColumn(m_listViewWnd, 1);
}

LRESULT ShortcutsConfigurationDlg::OnApply()
{
   for(int i=0; i<ListView_GetItemCount(m_listViewWnd); i++)
      GetItemHotkey(i)->Commit();
   
   //Apply succeeded
   return PSNRET_NOERROR;
}

void ShortcutsConfigurationDlg::OnClick(LPNMITEMACTIVATE lpnmitem)
{
   if (IsEditing())
      EndEdit();

   if ((lpnmitem->iItem == -1) || (lpnmitem->iSubItem != 1))
      return;

   BeginEdit(lpnmitem->iItem);
}   

void ShortcutsConfigurationDlg::OnRightClick(LPNMITEMACTIVATE lpnmitem)
{
	HMENU hMenu = Locale::GetInstance().LoadMenu(IDM_SHORTCUTS_CTXMENU);
	HMENU hPopupMenu = GetSubMenu(hMenu, 0);

   if (lpnmitem->iItem == -1)
      return;

   POINT pt = { lpnmitem->ptAction.x, lpnmitem->ptAction.y };
   ClientToScreen(m_listViewWnd, &pt);

   switch(TrackPopupMenu(hPopupMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON, 
                         pt.x, pt.y, 0, 
                         m_hDlg, NULL))
   {
   case IDC_CLEAR_SHORTCUT:
      SetItemShortcut(lpnmitem->iItem, 0);
      break;

   case IDC_RESET_SHORTCUT:
      SetItemShortcut(lpnmitem->iItem, GetItemHotkey(lpnmitem->iItem)->GetHotkey());
      break;

   default:
      break;
   }

   DestroyMenu(hMenu);
}

void ShortcutsConfigurationDlg::OnSetFocus()
{
   if (IsEditing())
      EndEdit();
}

// Message handler for the shortcuts settings page.
LRESULT CALLBACK ShortcutsConfigurationDlg::DlgProc(HWND hDlg, UINT message, WPARAM /*wParam*/, LPARAM lParam)
{
   ShortcutsConfigurationDlg * self;

	switch (message)
	{
	case WM_INITDIALOG:
      self = new ShortcutsConfigurationDlg(hDlg);
      SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)self);
		return TRUE;

   case WM_DESTROY:
      self = (ShortcutsConfigurationDlg*)GetWindowLongPtr(hDlg, DWLP_USER);
      delete self;
      return 0;

   case WM_NOTIFY:
      LPNMHDR pnmh = (LPNMHDR) lParam;
      switch (pnmh->code)
      {
      case NM_CLICK:
         if (GetDlgCtrlID(pnmh->hwndFrom) != IDC_SHORTCUTSLIST)
            break;

         self = (ShortcutsConfigurationDlg*)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnClick((LPNMITEMACTIVATE) lParam);
         break;

      case NM_RCLICK:
         if (GetDlgCtrlID(pnmh->hwndFrom) != IDC_SHORTCUTSLIST)
            break;

         self = (ShortcutsConfigurationDlg*)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnRightClick((LPNMITEMACTIVATE) lParam);
         break;

      case NM_SETFOCUS:
         if (GetDlgCtrlID(pnmh->hwndFrom) != IDC_SHORTCUTSLIST)
            break;

         self = (ShortcutsConfigurationDlg*)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnSetFocus();
         break;

      case PSN_KILLACTIVE:
         self = (ShortcutsConfigurationDlg*)GetWindowLongPtr(hDlg, DWLP_USER);
         if (self->IsEditing())
            self->EndEdit();
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, FALSE);
         return TRUE;

      case PSN_APPLY:
         self = (ShortcutsConfigurationDlg*)GetWindowLongPtr(hDlg, DWLP_USER);
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, self->OnApply());
         return TRUE; 
      }
      break;
	}
	return FALSE;
}
