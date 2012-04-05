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

#ifndef __SHAREDMENUBUFFER_H__
#define __SHAREDMENUBUFFER_H__

#define SHAREDMENUBUFFER_DEFAULT_LENGTH   2000

#define ITEMTYPE_LEN    sizeof(char)
#define ITEMID_LEN      sizeof(UINT_PTR)

class SharedMenuBuffer
{
public:
   SharedMenuBuffer(unsigned int length = SHAREDMENUBUFFER_DEFAULT_LENGTH);
   SharedMenuBuffer(DWORD processId, HANDLE filemapping);
   ~SharedMenuBuffer();

   inline HANDLE GetFileMapping()      { return m_hFileMapping; }

   /** Parse the content of the buffer and fill the menu consequently.
    */
   bool ReadMenu(HMENU hMenu, UINT (*filter)(UINT));

   /** Insert a menu item description in the buffer.
    */
   bool InsertMenu(UINT_PTR id, LPCSTR text, BOOL check);

   bool InsertSeparator()              { return InsertMenu(0, NULL, FALSE);  }

protected:
   HANDLE m_hFileMapping;
   LPVOID m_hViewPtr;
   char * m_pBuffer;

   char& ItemType(char * buffer)       { return *buffer; }
   UINT_PTR& ItemId(char * buffer)     { return *(UINT_PTR*)(buffer+ITEMTYPE_LEN); }
   char* ItemName(char * buffer)       { return buffer+ITEMTYPE_LEN+ITEMID_LEN; }
   int ItemLength(char * buffer)       { return strlen(ItemName(buffer))+1+ITEMTYPE_LEN+ITEMID_LEN; }

   enum {
      MENUITEM_NONE,
      MENUITEM_UNCHECKED,
      MENUITEM_CHECKED
   };
};

#endif /*__SHAREDMENUBUFFER_H__*/
