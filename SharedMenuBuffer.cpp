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
#include "SharedMenuBuffer.h"

SharedMenuBuffer::SharedMenuBuffer(unsigned int length)
{
   m_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, length, NULL);
   m_hViewPtr = NULL;
}

SharedMenuBuffer::SharedMenuBuffer(DWORD dwProcessId, HANDLE hFileMapping)
{
   HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwProcessId);
   DuplicateHandle(hProcess, hFileMapping, GetCurrentProcess(), &m_hFileMapping, FILE_MAP_ALL_ACCESS, FALSE, 0);
   CloseHandle(hProcess);
   m_hViewPtr = NULL;
}

SharedMenuBuffer::~SharedMenuBuffer()
{
   if (m_hViewPtr)
      UnmapViewOfFile(m_hViewPtr);
   CloseHandle(m_hFileMapping);
}

bool SharedMenuBuffer::ReadMenu(HMENU hMenu, UINT (*filter)(UINT))
{
   LPVOID viewPtr = MapViewOfFile(m_hFileMapping, FILE_MAP_READ, 0, 0, 0);
   char * buffer = (char*)viewPtr;
   bool result = true;

   if (viewPtr == NULL)
      return false;

   //Build the menu from the data in the buffer
   while(ItemType(buffer) != MENUITEM_NONE)
   {
      MENUITEMINFO menuitem;

      menuitem.cbSize = sizeof(MENUITEMINFO);
      menuitem.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
      menuitem.fType = (ItemId(buffer) == 0) ? MFT_SEPARATOR : MFT_STRING;
      menuitem.wID = filter(ItemId(buffer));
      menuitem.fState = (ItemType(buffer) == MENUITEM_CHECKED) ? MFS_CHECKED : MFS_UNCHECKED;
      menuitem.dwTypeData = ItemName(buffer);

      if (!InsertMenuItem(hMenu, (UINT)-1, TRUE, &menuitem))
      {
         result = false;
         break;
      }

      buffer += ItemLength(buffer);
   }

   UnmapViewOfFile(viewPtr);

   return result;
}

bool SharedMenuBuffer::InsertMenu(UINT_PTR id, LPCSTR text, BOOL check)
{
   //Create a view of the file if none yet
   if (m_hViewPtr == NULL)
   {
      m_hViewPtr = MapViewOfFile(m_hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
      m_pBuffer = (char*)m_hViewPtr;
   }

   if (m_hViewPtr == NULL)
      return false;

   //TODO: Check if there is enough space in buffer

   //Insert the data in the buffer
   ItemType(m_pBuffer) = (char)(check ? MENUITEM_CHECKED : MENUITEM_UNCHECKED);
   ItemId(m_pBuffer) = id;
   strcpy(ItemName(m_pBuffer), text ? text : "");

   //Move the insertion pointer
   m_pBuffer += ItemLength(m_pBuffer);
   ItemType(m_pBuffer) = MENUITEM_NONE;

   return true;
}

