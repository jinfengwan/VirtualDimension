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
#include "WallPaper.h"
#include "PlatformHelper.h"
#include "BackgroundColor.h"
#include "ExplorerWrapper.h"

WallPaper * WallPaper::m_activeWallPaper = NULL;
bool WallPaper::m_defaultWallpaperInit = false;
TCHAR WallPaper::m_defaultWallpaper[MAX_PATH];

WallPaper::WallPaper()
{
   LoadDefaultWallpaper();

   m_fileName = m_bmpFileName = NULL;
   SetImage("");
}

WallPaper::WallPaper(LPTSTR fileName)
{
   LoadDefaultWallpaper();

   m_fileName = m_bmpFileName = NULL;
   SetImage(fileName);
}

WallPaper::~WallPaper(void)
{
   if (m_fileName != m_bmpFileName)
   {
      DeleteFile(m_bmpFileName);
      delete m_bmpFileName;
   }

   //Restore default wallpaper if this is the last Wallpaper object
   if (m_activeWallPaper == this)
   {
      //Restore wallpaper set the old way, or remove it if using Active Desktop
      SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, SETWALLPAPER_DEFAULT, 0);

      //Restore active desktop wallpaper, if any
      if (explorerWrapper->HasActiveDesktop() && *m_defaultWallpaper != 0)
         explorerWrapper->GetActiveDesktop()->ApplyChanges(AD_APPLY_REFRESH);
   }
}

void WallPaper::LoadDefaultWallpaper()
{
   if (!m_defaultWallpaperInit)
   {
      WCHAR buffer[MAX_PATH];

      //Try to get default wallpaper using active desktop
      if (!explorerWrapper->BindActiveDesktop() ||
          explorerWrapper->GetActiveDesktop()->GetWallpaper(buffer, sizeof(buffer)/sizeof(WCHAR), 0) != S_OK ||
          WideCharToMultiByte(CP_OEMCP, 0, buffer, -1, m_defaultWallpaper, sizeof(m_defaultWallpaper), NULL, NULL) == 0)
      {
         //If we could not get the default wallpaper properly from active desktop, try to get it from system parameters...
         SystemParametersInfo(SPI_GETDESKWALLPAPER, sizeof(m_defaultWallpaper)/sizeof(TCHAR), m_defaultWallpaper, 0);
      }

      m_defaultWallpaperInit = true;
   }
}

void WallPaper::Activate()
{
   if (m_activeWallPaper == this)
      return;

   m_activeWallPaper = this;
   m_wallPaperLoader.LoadImageAsync(this);
}

/** Set the wallpaper.
 * Points to the path of the image to use, or an empty string for default wallpaper (the one set when VD was started),
 * or NULL to disable the wallpaper.
 */
void WallPaper::SetImage(LPTSTR fileName)
{
   if (m_fileName != m_bmpFileName)
   {
      DeleteFile(m_bmpFileName);
      delete m_bmpFileName;
   }

   m_useDefaultWallpaper = (fileName != NULL && *fileName == 0);
   if (m_useDefaultWallpaper)
      m_fileName = m_defaultWallpaper;
   else
      m_fileName = fileName;
   m_bmpFileName = NULL;   //lazy image loading: load it the first time it is used

   if (m_activeWallPaper == this)
      m_wallPaperLoader.LoadImageAsync(this);
}

void WallPaper::SetColor(COLORREF bkColor)
{
   if (bkColor == m_bkColor)
      return;

   m_bkColor = bkColor;

   if (m_activeWallPaper == this)
      m_wallPaperLoader.LoadImageAsync(this);
}

WallPaper::WallPaperLoader WallPaper::m_wallPaperLoader;

WallPaper::WallPaperLoader::WallPaperLoader()
{
   DWORD dwThreadId;

   m_hStopThread = CreateEvent(NULL, TRUE, FALSE, NULL);
   m_hQueueSem = CreateSemaphore(NULL, 0, 1000, NULL);
   m_hQueueMutex = CreateMutex(NULL, FALSE, NULL);
   m_hWallPaperLoaderThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc,
                                           this, 0, &dwThreadId);
}

WallPaper::WallPaperLoader::~WallPaperLoader()
{
   SetEvent(m_hStopThread);
   WaitForSingleObject(m_hWallPaperLoaderThread, INFINITE);

   CloseHandle(m_hStopThread);
   CloseHandle(m_hQueueMutex);
   CloseHandle(m_hQueueSem);

   CloseHandle(m_hWallPaperLoaderThread);
}

DWORD WINAPI WallPaper::WallPaperLoader::ThreadProc(LPVOID lpParameter)
{
   WallPaperLoader * self = (WallPaperLoader *)lpParameter;
   HANDLE handles[2] = { self->m_hQueueSem, self->m_hStopThread };
   TCHAR tempPath[MAX_PATH-14];
   bool changed = false;

   GetTempPath(MAX_PATH-14, tempPath);

   while(WaitForMultipleObjects( 2, handles, FALSE, INFINITE) == WAIT_OBJECT_0)
   {
      WaitForSingleObject(self->m_hQueueMutex, INFINITE);
      WallPaper * wallpaper = self->m_WallPapersQueue.front();
      self->m_WallPapersQueue.pop_front();
      ReleaseMutex(self->m_hQueueMutex);

		if (changed || !wallpaper->m_useDefaultWallpaper)
		{
		   //TODO: for default wallpaper, we should now check if it has not changed
			changed = !wallpaper->m_useDefaultWallpaper;

			if (wallpaper->m_bmpFileName)
			{
				if (wallpaper == m_activeWallPaper)
					SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, wallpaper->m_bmpFileName, 0);
			}
			else if (wallpaper->m_fileName)
			{
				if (strnicmp(wallpaper->m_fileName + strlen(wallpaper->m_fileName)-4, ".bmp", 4) == 0)
				{
					wallpaper->m_bmpFileName = wallpaper->m_fileName;

					if (wallpaper == m_activeWallPaper)
						SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, wallpaper->m_bmpFileName, 0);
				}
				else
				{
					IPicture * picture = PlatformHelper::OpenImage(wallpaper->m_fileName);
					if (!picture)
					{
						wallpaper->m_fileName = NULL;
						continue;
					}

					wallpaper->m_bmpFileName = new TCHAR[MAX_PATH];
					if ( (GetTempFileName(tempPath, "VDIMG", 0, wallpaper->m_bmpFileName) == 0) ||
						  (!PlatformHelper::SaveAsBitmap(picture, wallpaper->m_bmpFileName)) )
					{
						delete wallpaper->m_bmpFileName;
						wallpaper->m_bmpFileName = NULL;
						picture->Release();
						continue;
					}

					if (wallpaper == m_activeWallPaper)
						SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, wallpaper->m_bmpFileName, 0);

					picture->Release();
				}
			}
			else
				SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)"", 0);
		}

      // Set the background color
      BackgroundColor::GetInstance().SetColor(wallpaper->m_bkColor);
   }

   ExitThread(0);
}

void WallPaper::WallPaperLoader::LoadImageAsync(WallPaper * wallpaper)
{
   WaitForSingleObject(m_hQueueMutex, INFINITE);
   m_WallPapersQueue.push_back(wallpaper);
   ReleaseMutex(m_hQueueMutex);

   ReleaseSemaphore(m_hQueueSem, 1, NULL);
}
