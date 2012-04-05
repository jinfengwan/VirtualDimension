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
#include "platformhelper.h"
#include <ole2.h>
#include "Locale.h"
#include "Resource.h"

PlatformHelper instance;

DWORD (*PlatformHelper::GetWindowFileName)(HWND hWnd, LPTSTR lpFileName, int iBufLen) = NULL;
HMODULE PlatformHelper::hPSAPILib = NULL;
PlatformHelper::GetModuleFileNameEx_t * PlatformHelper::pGetModuleFileNameEx = NULL;

void (*PlatformHelper::AlphaBlend)(HDC hdcDest, int nXOriginDest, int nYOriginDest, 
                                   HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, 
                                   int nWidth, int nHeight, BYTE sourceAlpha) = NULL;
HMODULE PlatformHelper::hMSImg32Lib = NULL;
PlatformHelper::AlphaBlend_t * PlatformHelper::pAlphaBlend = NULL;

PlatformHelper::SetMenuInfo_t * PlatformHelper::SetMenuInfo = NULL;

#define HIMETRIC_INCH   2540 

PlatformHelper::PlatformHelper(void)
{
   hPSAPILib = LoadLibrary("psapi.dll");
   pGetModuleFileNameEx = (GetModuleFileNameEx_t *)GetProcAddress(hPSAPILib, "GetModuleFileNameExA");

   if (pGetModuleFileNameEx)
      GetWindowFileName = GetWindowFileNameNT;
   else
      GetWindowFileName = GetWindowFileName9x;

   hMSImg32Lib = LoadLibrary("msimg32.dll");
   pAlphaBlend = (AlphaBlend_t *)GetProcAddress(hMSImg32Lib, "AlphaBlend");

   if (pAlphaBlend)
      AlphaBlend = AlphaBlendMSImg32;
   else
      AlphaBlend = AlphaBlendEmul;

   SetMenuInfo = (SetMenuInfo_t*)GetProcAddress(GetModuleHandle("User32.dll"), "SetMenuInfo");
   if (SetMenuInfo == NULL)
      SetMenuInfo = (SetMenuInfo_t*)SetMenuInfoDummy;
}

PlatformHelper::~PlatformHelper(void)
{
   FreeLibrary(hMSImg32Lib);
   FreeLibrary(hPSAPILib);
}

DWORD PlatformHelper::GetWindowFileNameNT(HWND hWnd, LPTSTR lpFileName, int nBufLen)
{
   DWORD pId;
   HINSTANCE hInstance;
   HANDLE hProcess;
   DWORD res;

   hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
   GetWindowThreadProcessId(hWnd, &pId);
   hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
   res = pGetModuleFileNameEx(hProcess, hInstance, lpFileName, nBufLen);
   CloseHandle(hProcess);

   return res;
}

DWORD PlatformHelper::GetWindowFileName9x(HWND, LPTSTR lpFileName, int nBufLen)
{
   if (nBufLen > 0)
      *lpFileName = '\000';
   return 0;
}

IPicture * PlatformHelper::OpenImage(LPCTSTR fileName)
{
   IPicture * picture = NULL;
   HGLOBAL hGlobal;
   IStream* pStream;
   DWORD dwSize;

   if (IS_INTRESOURCE(fileName))
   {
      HRSRC hrsrc = FindResource(NULL, fileName, MAKEINTRESOURCE(300));
      if (!hrsrc)
         return NULL;

      dwSize = SizeofResource(NULL, hrsrc);

      hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwSize);
	   if(!hGlobal)
         return NULL;

      HGLOBAL hres = LoadResource(NULL, hrsrc);
      void * pResData = LockResource(hres);

      void * pData = GlobalLock(hGlobal);
      memcpy(pData, pResData, dwSize);
	   GlobalUnlock(hGlobal);
   }
   else
   {
      HANDLE hFile = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (!hFile)
         return NULL;

      dwSize = GetFileSize(hFile, NULL);

      hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwSize);
	   if(!hGlobal)
      {
         CloseHandle(hFile);
         return NULL;
      }

	   void * pData = GlobalLock(hGlobal);
      DWORD dwNbRead;
      ReadFile(hFile, pData, dwSize, &dwNbRead, NULL);
	   GlobalUnlock(hGlobal);

      CloseHandle(hFile);

      if (dwNbRead < dwSize)
      {
         FreeResource(hGlobal);
         return NULL;
      }
   }

	if(CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) == S_OK)
   {
		if(OleLoadPicture(pStream, dwSize, FALSE, IID_IPicture, (LPVOID *)&picture) != S_OK)
         picture = NULL;

	   pStream->Release();  
	}
	FreeResource(hGlobal);

	return picture;
}

bool PlatformHelper::SaveAsBitmap(IPicture * picture, LPTSTR fileName)
{
	bool bResult = false;
	ILockBytes *Buffer = 0;
	IStorage   *pStorage = 0;
	IStream    *FileStream = 0;
	BYTE	   *BufferBytes;
	STATSTG		BytesStatistics;
	DWORD		OutData;
	long		OutStream;
   HANDLE		BitmapFile;
	double		SkipFloat = 0;
	DWORD		ByteSkip = 0;
	_ULARGE_INTEGER RealData;

	CreateILockBytesOnHGlobal(NULL, TRUE, &Buffer); // Create ILockBytes Buffer

	HRESULT hr = StgCreateDocfileOnILockBytes(Buffer,
				 STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, &pStorage);

	hr = pStorage->CreateStream(L"PICTURE",
		 STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, 0, &FileStream);

	picture->SaveAsFile(FileStream, TRUE, &OutStream); // Copy Data Stream
	FileStream->Release();
	pStorage->Release();
	Buffer->Flush(); 

	// Get Statistics For Final Size Of Byte Array
	Buffer->Stat(&BytesStatistics, STATFLAG_NONAME);

	// Cut UnNeeded Data Coming From SaveAsFile() (Leave Only "Pure" Picture Data)
	SkipFloat = (double(OutStream) / 512); // Must Be In a 512 Blocks...
	if(SkipFloat > DWORD(SkipFloat)) 
      ByteSkip = (DWORD)SkipFloat + 1;
	else 
      ByteSkip = (DWORD)SkipFloat;
	ByteSkip = ByteSkip * 512; // Must Be In a 512 Blocks...
	
	// Find Difference Between The Two Values
	ByteSkip = (DWORD)(BytesStatistics.cbSize.QuadPart - ByteSkip);

	// Allocate Only The "Pure" Picture Data
	RealData.LowPart = 0;
	RealData.HighPart = 0;
	RealData.QuadPart = ByteSkip;
	BufferBytes = (BYTE*)malloc(OutStream);
	if(BufferBytes == NULL)
   {  // Memory allocation failed
      return false;
	}

	Buffer->ReadAt(RealData, BufferBytes, OutStream, &OutData);

   BitmapFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
   if(BitmapFile != INVALID_HANDLE_VALUE)
	{
      DWORD nbWritten;
      WriteFile(BitmapFile, BufferBytes, OutData, &nbWritten, NULL);
      CloseHandle(BitmapFile);
	   bResult = true;
	}
	else // Write File Failed...
	{
      LPVOID lpMsgBuf;
		LPTSTR error;
		locGetString(error, IDS_ERROR);
      if (FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         (LPTSTR) &lpMsgBuf, 0, NULL))
         MessageBox( NULL, (LPCTSTR)lpMsgBuf, error, MB_OK | MB_ICONINFORMATION );

      LocalFree( lpMsgBuf );
	   bResult = false;
	}
	
	Buffer->Release();
	free(BufferBytes);

	return bResult;
}

void PlatformHelper::AlphaBlendMSImg32(HDC hdcDest, int nXOriginDest, int nYOriginDest, 
                                       HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, 
                                       int nWidth, int nHeight, BYTE sourceAlpha)
{
   BLENDFUNCTION bf;
   bf.AlphaFormat = 0;
   bf.BlendFlags = 0;
   bf.BlendOp = AC_SRC_OVER;
   bf.SourceConstantAlpha = sourceAlpha;

   pAlphaBlend(hdcDest, nXOriginDest, nYOriginDest, nWidth, nHeight, hdcSrc, 
               nXOriginSrc, nYOriginSrc, nWidth, nHeight, bf);
}

void PlatformHelper::AlphaBlendEmul(HDC hdcDest, int nXOriginDest, int nYOriginDest, 
                                    HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, 
                                    int nWidth, int nHeight, BYTE sourceAlpha)
{
   const BYTE otherAlpha = 255-sourceAlpha;
   for(int x = 0; x < nWidth; x++)
      for(int y = 0; y < nHeight; y++)
      {
         COLORREF dst = GetPixel(hdcDest, x+nXOriginDest, y+nYOriginDest);
         COLORREF src = GetPixel(hdcSrc, x+nXOriginSrc, y+nYOriginSrc);
         COLORREF res = RGB((GetRValue(dst) * sourceAlpha + GetRValue(src) * otherAlpha) >> 8,
                            (GetGValue(dst) * sourceAlpha + GetGValue(src) * otherAlpha) >> 8,
                            (GetBValue(dst) * sourceAlpha + GetBValue(src) * otherAlpha) >> 8);
         SetPixel(hdcDest, x+nXOriginDest, y+nYOriginDest, res);
      }
}

void PlatformHelper::CustomDrawIPicture(IPicture * picture, LPDRAWITEMSTRUCT lpDrawItem, bool resize)
{
   LPRECT rect = &lpDrawItem->rcItem;
   OLE_XSIZE_HIMETRIC width;
   OLE_YSIZE_HIMETRIC height;
   LONG x;
   LONG y;
   LONG nWidth;
   LONG nHeight;
   
   picture->get_Width(&width);
   picture->get_Height(&height);

   if (resize)
   {
      //Get target dimension
      nWidth = rect->right - rect->left;
      nHeight = rect->bottom - rect->top;
   }
   else
   {
      //Get image dimensions (convert to device units)
      nWidth  = MulDiv(width, GetDeviceCaps(lpDrawItem->hDC, LOGPIXELSX), HIMETRIC_INCH);
      nHeight = MulDiv(height, GetDeviceCaps(lpDrawItem->hDC, LOGPIXELSY), HIMETRIC_INCH);
   }

   //Center picture
   x = (rect->left + rect->right - nWidth) / 2;
   y = (rect->top + rect->bottom - nHeight) / 2;

   picture->Render(lpDrawItem->hDC, x, y, nWidth, nHeight, 0, height, width, -height, rect);
}
