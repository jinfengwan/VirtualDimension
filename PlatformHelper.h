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

#ifndef __PLATFORMHELPER_H__
#define __PLATFORMHELPER_H__

#include <objbase.h>
#include <oaidl.h>
#include <ocidl.h>
#include <olectl.h>

#ifdef __GNUC__

#ifdef UNICODE
#define TTM_SETTITLE TTM_SETTITLEW
#else
#define TTM_SETTITLE TTM_SETTITLEA
#endif

#define UDM_SETRANGE32 (WM_USER+111)

#define TBM_SETBUDDY (WM_USER+32)

#define MNS_CHECKORBMP 0x04000000
#define SETWALLPAPER_DEFAULT ((LPWSTR)-1)
#define AD_APPLY_REFRESH 0x4

#define LPWALLPAPEROPT void*
#define LPCWALLPAPEROPT const void*
#define LPCOMPONENTSOPT void*
#define LPCCOMPONENTSOPT const void*
#define LPCOMPONENT void*
#define LPCCOMPONENT const void*

#undef INTERFACE
#define INTERFACE IActiveDesktop
DECLARE_INTERFACE_(IActiveDesktop, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
   STDMETHOD (ApplyChanges)(THIS_ DWORD dwFlags) PURE;
   STDMETHOD (GetWallpaper)(THIS_ LPWSTR pwszWallpaper, UINT cchWallpaper, DWORD dwReserved) PURE;
   STDMETHOD (SetWallpaper)(THIS_ LPCWSTR pwszWallpaper, DWORD dwReserved) PURE;
   STDMETHOD (GetWallpaperOptions)(THIS_ LPWALLPAPEROPT pwpo, DWORD dwReserved) PURE;
   STDMETHOD (SetWallpaperOptions)(THIS_ LPCWALLPAPEROPT pwpo, DWORD dwReserved) PURE;
   STDMETHOD (GetPattern)(THIS_ LPWSTR pwszPattern, UINT cchPattern, DWORD dwReserved) PURE;
   STDMETHOD (SetPattern)(THIS_ LPCWSTR pwszPattern, DWORD dwReserved) PURE;
   STDMETHOD (GetDesktopItemOptions)(THIS_ LPCOMPONENTSOPT pco, DWORD dwReserved) PURE;
   STDMETHOD (SetDesktopItemOptions)(THIS_ LPCCOMPONENTSOPT pco, DWORD dwReserved) PURE;
   STDMETHOD (AddDesktopItem)(THIS_ LPCCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (AddDesktopItemWithUI)(THIS_ HWND hwnd, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (ModifyDesktopItem)(THIS_ LPCCOMPONENT pcomp, DWORD dwFlags) PURE;
   STDMETHOD (RemoveDesktopItem)(THIS_ LPCCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (GetDesktopItemCount)(THIS_ LPINT lpiCount, DWORD dwReserved) PURE;
   STDMETHOD (GetDesktopItem)(THIS_ int nComponent, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (GetDesktopItemByID)(THIS_ ULONG_PTR dwID, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (GenerateDesktopItemHtml)(THIS_ LPCWSTR pwszFileName, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (AddUrl)(THIS_ HWND hwnd, LPCWSTR pszSource, LPCOMPONENT pcomp, DWORD dwFlags) PURE;
   STDMETHOD (GetDesktopItemBySource)(THIS_ LPCWSTR pwszSource, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
};
typedef IActiveDesktop *LPIACTIVEDESKTOP;
extern "C" const GUID CLSID_ActiveDesktop;	//'75048700-EF F- D0-9888-006097DEACF9}'
extern "C" const GUID IID_IActiveDesktop;		//D1:$52502EE0; D2:$EC80; D3:$11D0; D4:($89, $AB, $00, $C0, $4F, $C2, $97, $2D));

#undef INTERFACE
#define INTERFACE ITaskbarList
DECLARE_INTERFACE_(ITaskbarList, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

   STDMETHOD(ActivateTab)(THIS_ HWND) PURE;
   STDMETHOD(AddTab)(THIS_ HWND) PURE;
   STDMETHOD(DeleteTab)(THIS_ HWND) PURE;
   STDMETHOD(HrInit)(THIS) PURE;
   STDMETHOD(SetActiveAtl)(THIS_ HWND) PURE;
};
extern "C" const GUID CLSID_TaskbarList;
extern "C" const GUID IID_ITaskbarList;

#endif /*__GNUC__*/

#ifdef DEBUG
#define TRACE(str) OutputDebugString(str)
#else
#define TRACE(str) 
#endif

class PlatformHelper
{
public:
   PlatformHelper(void);
   ~PlatformHelper(void);

   static DWORD (*GetWindowFileName)(HWND hWnd, LPTSTR lpFileName, int iBufLen);
   static void (*AlphaBlend)(HDC hdcDest, int nXOriginDest, int nYOriginDest, 
                             HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, 
                             int nWidth, int nHeight, BYTE sourceAlpha);
   
   static IPicture * OpenImage(LPCTSTR fileName);
   static bool SaveAsBitmap(IPicture * picture, LPTSTR fileName);

   static void CustomDrawIPicture(IPicture * picture, LPDRAWITEMSTRUCT lpDrawItem, bool resize = true);

   typedef BOOL WINAPI SetMenuInfo_t(HMENU, LPCMENUINFO);
	static SetMenuInfo_t * SetMenuInfo;

protected:
   typedef DWORD WINAPI GetModuleFileNameEx_t(HANDLE,HMODULE,LPTSTR,DWORD);

   static HMODULE hPSAPILib;
   static GetModuleFileNameEx_t * pGetModuleFileNameEx;

   static DWORD GetWindowFileNameNT(HWND hWnd, LPTSTR lpFileName, int iBufLen);
   static DWORD GetWindowFileName9x(HWND hWnd, LPTSTR lpFileName, int iBufLen);


   typedef BOOL WINAPI AlphaBlend_t(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);

   static HMODULE hMSImg32Lib;
   static AlphaBlend_t * pAlphaBlend;

   static void AlphaBlendMSImg32(HDC hdcDest, int nXOriginDest, int nYOriginDest, 
                                 HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, 
                                 int nWidth, int nHeight, BYTE sourceAlpha);
   static void AlphaBlendEmul(HDC hdcDest, int nXOriginDest, int nYOriginDest, 
                              HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, 
                              int nWidth, int nHeight, BYTE sourceAlpha);

   static BOOL WINAPI SetMenuInfoDummy(HMENU /*hmenu*/, LPCMENUINFO /*lpcmi*/)    { return 0; }
};

#endif /*__PLATFORMHELPER_H__*/
