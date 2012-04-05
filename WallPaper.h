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

#ifndef __WALLPAPER_H__
#define __WALLPAPER_H__

#include <list>

using namespace std;

class WallPaper
{
public:
   WallPaper();
   WallPaper(LPTSTR fileName);
   ~WallPaper(void);

   void Activate();
   void SetImage(LPTSTR fileName);
   void SetColor(COLORREF bkColor);

	static void RefreshDefaultWallpaper()	{ m_defaultWallpaperInit = false; LoadDefaultWallpaper(); }
   static LPCTSTR GetDefaultWallpaper()   { return m_defaultWallpaper; }

protected:
	bool m_useDefaultWallpaper;
   LPTSTR m_fileName;
   LPTSTR m_bmpFileName;

   COLORREF m_bkColor;

   class WallPaperLoader
   {
   public:
      WallPaperLoader();
      ~WallPaperLoader();
      void LoadImageAsync(WallPaper * wallpaper);

   protected:
      static DWORD WINAPI ThreadProc(LPVOID lpParameter);

      list<WallPaper *> m_WallPapersQueue;
      HANDLE m_hStopThread;
      HANDLE m_hQueueSem;
      HANDLE m_hQueueMutex;
      HANDLE m_hWallPaperLoaderThread;
   };

   static void LoadDefaultWallpaper();

   static WallPaper * m_activeWallPaper;
   static WallPaperLoader m_wallPaperLoader;

   static bool m_defaultWallpaperInit;
   static TCHAR m_defaultWallpaper[MAX_PATH];
};

#endif /*__WALLPAPER_H__*/
