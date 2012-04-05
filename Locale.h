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

#ifndef __LOCALE_H__
#define __LOCALE_H__

#include <malloc.h>

#include "StdString.h"
typedef CStdString String;

class Locale
{
public:
	Locale(void);
	~Locale(void);

	bool SetLanguage(int langcode); // loads anotehr instance for this language
	int GetLanguage(void) const         { return m_currentLangCode; }

	static Locale& GetInstance()			{ return m_instance; }

	operator HINSTANCE()						{ return GetResDll(); }
	HINSTANCE GetResDll()					{ return m_resDll; }

	HMENU LoadMenu(UINT uID)				{ return LoadMenu(MAKEINTRESOURCE(uID)); }
	HMENU LoadMenu(LPCTSTR lpMenuName)	{ return ::LoadMenu(m_resDll, lpMenuName); }

	String GetString(UINT uID)				{ return GetString(m_resDll, uID); }
	int GetStringSize(UINT uID)				{ return GetStringSize(m_resDll, uID); }
	LPTSTR GetString(UINT uID, LPTSTR str, int len)	{ LoadString(*this, uID, str, len); return str; }
	static String GetString(HINSTANCE hinst, UINT uID);
	static int GetStringSize(HINSTANCE hinst, UINT uID);

	int MessageBox(HWND hWnd, UINT uIdText, UINT uIdCaption, UINT uType);

   static int GetLanguageCode(LPTSTR str);

protected:
	static Locale m_instance;

   int m_currentLangCode;
	HINSTANCE m_resDll;
};

#define locMessageBox(hwnd, uIdText, uIdCaption, uType)					\
	Locale::GetInstance().MessageBox(hwnd, uIdText, uIdCaption, uType)

/** An efficient macro to retrieve a resource string.
 * This macro is quite efficient, as it uses memory allocated from the stack.
 * However, this can lead to some issues (like stack overflow/exaustion if it
 * is called multiple times in the same context. Thus, care must be taken to
 * ensure no such situations can arise.
 *
 * It also adds a limitation on the size of the returned string: it is fixed
 * to 0xffff at the moment, which should be enough with regard to the stack
 * but this number can be increased at will. The only requirement is that this
 * number should be greater than the space allocated for the string resource.
 */
#define locGetString(str, uId)													\
	(str = (LPTSTR)_alloca(Locale::GetInstance().GetStringSize(uId)),	\
	 Locale::GetInstance().GetString(uId, (LPTSTR)str, 0xffff))

class LocalesIterator
{
public:
	LocalesIterator();
	~LocalesIterator();
	bool GetNext();

	/** Get the language name, and optionally the icon.
	 * hIcon can be NULL if the icon is not needed. It is the responsibility
	 * of the caller to free the icon once he doesn't need it anymore.
	 */
	String GetLanguage(HICON * hSmIcon, HICON * hLgIcon);

	/** Get the language code associated with this language.
	 * Language code is a unique representation of the language, and it fits
	 * in 10 bits (0-1023). 0 is a reserved value (used for auto-detection).
	 */
	int GetLanguageCode();

protected:
	WIN32_FIND_DATA m_FindFileData;
	HANDLE m_hFind;
};

#endif /*__LOCALE_H__*/
