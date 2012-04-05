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
#include "Config.h"

using namespace Config;

unsigned int Group::LoadSetting(const Setting<LPTSTR> &setting, LPTSTR buffer, unsigned int length, const StringSetting& /*type*/)
{
   DWORD size;

   size = LoadString(setting.m_name, NULL);
   if (!size)
      size = _tcslen(setting.m_default);

   if (buffer &&
       (size > length || LoadString(setting.m_name, buffer) == 0))
   {
      size = _tcslen(setting.m_default);
      strncpy(buffer, setting.m_default, length-1);
      buffer[length-1] = 0;
   }

   return size;
}

bool RegistryGroup::Open(HKEY parent, const char * path, bool create)
{
   if (m_opened)
      Close();

   if (create)
      m_opened = RegCreateKeyEx(parent, path, 
                  0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, 
                  &m_regKey, NULL) == ERROR_SUCCESS;
   else
      m_opened = RegOpenKeyEx(parent, path, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                  &m_regKey) == ERROR_SUCCESS;

   return m_opened;
}

void RegistryGroup::Close()
{
   if (m_opened)
      RegCloseKey(m_regKey);
   m_opened = false;
}

Group * RegistryGroup::GetSubGroup(const char * path)
{
   RegistryGroup * m_subgroup = new RegistryGroup();

   if (!m_opened || !m_subgroup->Open(m_regKey, path))
   {
      delete m_subgroup;
      m_subgroup = NULL;
   }

   return m_subgroup;
}

DWORD RegistryGroup::LoadDWord(const char * entry, DWORD defVal)
{
   DWORD size;
   DWORD val;
   DWORD type;

   if ( (!m_opened) || 
        (RegQueryValueEx(m_regKey, entry, NULL, &type, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(val)) || 
        (type != REG_DWORD) ||
        (RegQueryValueEx(m_regKey, entry, NULL, NULL, (LPBYTE)&val, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the value from registry --> set default value
      val = defVal;
   }

   return val;
}

void RegistryGroup::SaveDWord(const char * entry, DWORD value)
{
   if (m_opened)
      RegSetValueEx(m_regKey, entry, 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
}

bool RegistryGroup::LoadBinary(const char * entry, LPBYTE buffer, DWORD length, LPBYTE defval)
{
   DWORD size;
   DWORD type;
   bool res;

   res = (m_opened) &&
         (RegQueryValueEx(m_regKey, entry, NULL, &type, NULL, &size) == ERROR_SUCCESS) &&
         (size == length) && 
         (type == REG_BINARY) &&
         (RegQueryValueEx(m_regKey, entry, NULL, NULL, buffer, &size) == ERROR_SUCCESS);

   if (!res)
   {
      // Cannot load the value from registry --> set default value
      memcpy(buffer, defval, length);
   }

   return res;
}

void RegistryGroup::SaveBinary(const char * entry, LPBYTE buffer, DWORD length)
{
   if (m_opened)
      RegSetValueEx(m_regKey, entry, 0, REG_BINARY, buffer, length);
}

unsigned int RegistryGroup::LoadString(const char * entry, LPTSTR buffer)
{
   DWORD size;
   DWORD type;

   if ( !m_opened || 
        RegQueryValueEx(m_regKey, entry, NULL, &type, NULL, &size) != ERROR_SUCCESS ||
        type != REG_SZ )
      size = 0;

   if ( size &&
        buffer &&
        RegQueryValueEx(m_regKey, entry, NULL, NULL, (LPBYTE)buffer, &size) != ERROR_SUCCESS )
      size = 0;

   return size;
}

void RegistryGroup::SaveString(const char * entry, LPTSTR buffer)
{
   DWORD len;

   len = (DWORD)((_tcslen(buffer)+1) * sizeof(TCHAR));
   if (m_opened)
      RegSetValueEx(m_regKey, entry, 0, REG_SZ, (LPBYTE)buffer, len);
}

bool RegistryGroup::RemoveEntry(LPTSTR entry)
{
   return m_opened && RegDeleteValue(m_regKey, entry);
}

bool RegistryGroup::RemoveGroup(LPTSTR group)
{
   return m_opened && RegDeleteKey(m_regKey, group);
}

BOOL RegistryGroup::EnumEntry(DWORD dwIndex, LPTSTR lpName, LPDWORD lpcName)
{
   return m_opened && 
      RegEnumValue(m_regKey, dwIndex, lpName, lpcName, NULL, NULL, NULL, NULL)/*==ERROR_SUCCESS*/ != ERROR_NO_MORE_ITEMS;
}

BOOL RegistryGroup::EnumGroup(DWORD dwIndex, LPTSTR lpName, DWORD cName)
{
   return m_opened && 
      RegEnumKey(m_regKey, dwIndex, lpName, cName)==ERROR_SUCCESS;
}
