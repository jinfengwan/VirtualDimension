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
#include "Resource.h"
#include "Settings.h"
#include "Locale.h"


Locale Locale::m_instance;

Locale::Locale(void): m_currentLangCode(0), m_resDll(NULL)
{
    Settings settings;
    int code;

    //Load last used language
    code = settings.LoadSetting(Settings::LanguageCode);

    //Try to load that language
    if (!SetLanguage(code))
    {
        // by default, it will stay with ENglish one
        if (!SetLanguage(Locale::GetLanguageCode("EN")))
        {
            //TODO: try to find another resource library (maybe there is only langFR.dll)
            //TODO: if that fails, display error message (hardcoded in any language), and terminate application immediatly
        }
    }
}

Locale::~Locale(void)
{
	::FreeLibrary(m_resDll);
}

bool Locale::SetLanguage(int langcode)
{
    bool bResult = false;

    if (langcode == 0)
    {
        //TODO: try to auto-detect language
    }

    if (langcode)
    {
        char buffer[16];
        sprintf(buffer,"lang%c%c.dll", 'A'-1+(langcode & 0x1f), 'A'-1+((langcode >> 5) & 0x1f));
        HINSTANCE hInstTemporary = (HINSTANCE)::LoadLibrary(buffer);
        if ( hInstTemporary != NULL)
        {
            //Change the resource library used
            bResult = true;
            if (m_resDll)
               ::FreeLibrary(m_resDll);
            m_resDll = hInstTemporary;
            m_currentLangCode = langcode;

            //Save the new language
            Settings    settings;
            settings.SaveSetting(Settings::LanguageCode,langcode);
        }
    }

    return bResult;
}


String Locale::GetString(HINSTANCE hinst, UINT uID)
{
	String str;
	int size = GetStringSize(hinst, uID);

	//Load the resource
	LoadString(hinst, uID, str.GetBuffer(size), size/sizeof(TCHAR));
	str.ReleaseBuffer();

	return str;
}

int Locale::GetStringSize(HINSTANCE hinst, UINT uID)
{
	HRSRC hrsrc = FindResource(hinst, MAKEINTRESOURCE((uID>>4)+1), RT_STRING);
	if (hrsrc == NULL)
		return 0;
	else
		return SizeofResource(hinst, hrsrc);
}

int Locale::MessageBox(HWND hWnd, UINT uIdText, UINT uIdCaption, UINT uType)
{
	LPTSTR text;
	LPTSTR title;
	locGetString(text, uIdText);
	locGetString(title, uIdCaption);
	return ::MessageBox(hWnd, text, title, uType);
}

int Locale::GetLanguageCode(LPTSTR str)
{
   char first = str[0];
   char second = str[1];
   if (first >= 'a' && first <= 'z')
      first -= 'a' - 'A';
   if (second >= 'a' && second <= 'z')
      second -= 'a' - 'A';
	return ((first + 1 - 'A') & 0x1f) | (((second + 1 - 'A') & 0x1f) << 5);
}

LocalesIterator::LocalesIterator()
{
	m_hFind = INVALID_HANDLE_VALUE;
}

LocalesIterator::~LocalesIterator()
{
	if (m_hFind != INVALID_HANDLE_VALUE)
		FindClose(m_hFind);
}

bool LocalesIterator::GetNext()
{
	*m_FindFileData.cFileName = 0;	//empty the file name
	if (m_hFind == INVALID_HANDLE_VALUE)
	{
		char path[MAX_PATH];
		char * ptr;

		//Build the mask, including path
		GetModuleFileName(NULL, path, MAX_PATH);
		ptr = strrchr(path, '\\');
		if (ptr)
			*(ptr+1) = 0;	//strip file name (but keep back-slash)
		else
			*path = 0;		//strange path -> search in current directory
		strcat(path, "lang*.dll");

		//Begin the search
		m_hFind = FindFirstFile(path, &m_FindFileData);
		return m_hFind != INVALID_HANDLE_VALUE;
	}
	else
	{
		return FindNextFile(m_hFind, &m_FindFileData) != 0;
	}
}

String LocalesIterator::GetLanguage(HICON * hSmIcon, HICON * hLgIcon)
{
	String res;
	HINSTANCE hInst = LoadLibrary(m_FindFileData.cFileName);
	if (hInst)
	{
		res = Locale::GetString(hInst, IDS_LANGUAGE);
		if (hLgIcon)
			*hLgIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDS_LANGUAGE), IMAGE_ICON, 32, 32, LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS);
		if (hSmIcon)
			*hSmIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDS_LANGUAGE), IMAGE_ICON, 16, 16, LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS);
		FreeLibrary(hInst);
	}
	return res;
}

int LocalesIterator::GetLanguageCode()
{
	return Locale::GetLanguageCode(m_FindFileData.cFileName+4);
}
