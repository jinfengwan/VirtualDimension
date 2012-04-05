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
#include "LinkControl.h"
#include "VirtualDimension.h"
#include <shellapi.h>
#include <windowsx.h>

static ATOM RegisterHyperLinkClass(HINSTANCE hInstance);
static LRESULT CALLBACK	HyperLinkWndProc(HWND, UINT, WPARAM, LPARAM);
static HFONT GetHyperLinkFont(HWND hWnd, HDC hdc);
static void ActivateLink(HWND hWnd);
static void ResizeToText(HWND hWnd, LPTSTR text);

void InitHyperLinkControl()
{
   RegisterHyperLinkClass(vdWindow);
}

typedef struct HLControl {
   TCHAR m_text[MAX_PATH];
   TCHAR m_link[MAX_PATH];
   bool m_focused;
} HLControl;

static ATOM RegisterHyperLinkClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wcex.lpfnWndProc	= (WNDPROC)HyperLinkWndProc;
	wcex.cbClsExtra   = sizeof(HFONT);
   wcex.cbWndExtra	= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
   wcex.hCursor		= LoadCursor(NULL, IDC_HAND);
   wcex.hbrBackground= (HBRUSH)(COLOR_3DFACE+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName= "HyperLinkControl";
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

static HFONT GetHyperLinkFont(HWND hWnd, HDC hdc)
{
   HFONT hf;

   hf = (HFONT)GetClassLongPtr(hWnd, 0);

   if (hf == NULL)
   {
      TEXTMETRIC tm;
      LOGFONT lf;

      SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
      GetTextMetrics(hdc, &tm);

      lf.lfHeight = tm.tmHeight;
      lf.lfWidth = 0;
      lf.lfEscapement = 0;
      lf.lfOrientation = 0;
      lf.lfWeight = tm.tmWeight;
      lf.lfItalic = tm.tmItalic;
      lf.lfUnderline = TRUE;
      lf.lfStrikeOut = tm.tmStruckOut;
      lf.lfCharSet = tm.tmCharSet;
      lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
      lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
      lf.lfQuality = DEFAULT_QUALITY;
      lf.lfPitchAndFamily = tm.tmPitchAndFamily;
      GetTextFace(hdc, LF_FACESIZE, lf.lfFaceName);

      hf = CreateFontIndirect(&lf);

      SetClassLongPtr(hWnd, 0, (LONG_PTR)hf);
   }

   return hf;
}

static void ActivateLink(HWND hWnd)
{
   //HLControl * hlCtrl = (HLControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

   //ShellExecute(hWnd, "open", hlCtrl->m_link, NULL, NULL, SW_SHOWNORMAL);

   SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), STN_CLICKED), 0);
}

static void ResizeToText(HWND hWnd, LPTSTR text)
{
   SIZE size;
   HDC hdc = GetDC(hWnd);

   SelectObject(hdc, GetHyperLinkFont(hWnd, hdc));
   GetTextExtentPoint32(hdc, text, strlen(text), &size);

   ReleaseDC(hWnd, hdc);

   SetWindowPos(hWnd, NULL, 0, 0, size.cx+2, size.cy+2, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
}

static void RepaintBackground(HWND hWnd)
{
   RECT rect;
   HWND hParentWnd;

   if (GetWindowExStyle(hWnd) & WS_EX_TRANSPARENT)
   {
      hParentWnd = GetParent(hWnd);
      GetWindowRect(hWnd, &rect);
      ScreenToClient(hParentWnd, (POINT*)&rect);
      InvalidateRect(hParentWnd, &rect, TRUE);
      UpdateWindow(hParentWnd);
   }
}

static LRESULT CALLBACK HyperLinkWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   HLControl * hlCtrl;

   switch (message)
	{
   case WM_CREATE:
      hlCtrl = (HLControl *)malloc(sizeof(HLControl));
      memset(hlCtrl, 0, sizeof(HLControl));
      hlCtrl->m_focused = false;
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)hlCtrl);
      
      //Setup initial text. We must bypass our own implementation of WM_GETTEXT,
      //so we must call directoy DefWindowProc().
      DefWindowProc(hWnd, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)hlCtrl->m_text);
      ResizeToText(hWnd, hlCtrl->m_text);
      break;

   case WM_DESTROY:
      hlCtrl = (HLControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      free(hlCtrl);
      break;

   case WM_GETDLGCODE:
      return DLGC_DEFPUSHBUTTON;

   case WM_KEYDOWN:
      if ((wParam != VK_SELECT) && (wParam != VK_SPACE))
         break;
   case WM_LBUTTONDOWN:
      SetFocus(hWnd);
      ActivateLink(hWnd);
      break;

   case WM_SETTEXT:
      hlCtrl = (HLControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      strncpy(hlCtrl->m_text, (LPTSTR)lParam, MAX_PATH);
      ResizeToText(hWnd, hlCtrl->m_text);
      RepaintBackground(hWnd);
      break;

   case WM_GETTEXT:
      hlCtrl = (HLControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      strncpy((LPTSTR)lParam, hlCtrl->m_text, (int)wParam);
      break;

   case WM_SETFOCUS:
      hlCtrl = (HLControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      hlCtrl->m_focused = true;
      InvalidateRect(hWnd, NULL, FALSE);
      RepaintBackground(hWnd);
      break;

   case WM_KILLFOCUS:
      hlCtrl = (HLControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      hlCtrl->m_focused = false;
      InvalidateRect(hWnd, NULL, TRUE);
      RepaintBackground(hWnd);
      break;

   case WM_ERASEBKGND:
      if (GetWindowExStyle(hWnd) & WS_EX_TRANSPARENT)
         return TRUE;   //skip drawing background if the window is transparent
      break;

   case WM_PAINT:
      {
         PAINTSTRUCT ps;
	      HDC hdc;
         RECT rect;
         hlCtrl = (HLControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

         GetClientRect(hWnd, &rect);

		   hdc = BeginPaint(hWnd, &ps);
         SelectObject(hdc, GetHyperLinkFont(hWnd, hdc));
         SetTextColor(hdc, RGB(0,0,255));
         SetBkMode(hdc, TRANSPARENT);
         DrawTextEx(hdc, hlCtrl->m_text, -1, &rect, DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX, NULL);

         //Draw dot-line rectangle around the control if it is selected
         if (hlCtrl->m_focused)
         {
            LOGBRUSH lb;

            lb.lbStyle = BS_SOLID;
            lb.lbColor = RGB(0,0,0);
            lb.lbHatch = 0;

            HPEN hpen = ExtCreatePen(PS_COSMETIC | PS_ALTERNATE, 1, &lb, 0, NULL);

            SelectObject(hdc, hpen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
            DeleteObject(hpen);
         }

		   EndPaint(hWnd, &ps);
      }
		break;

   default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
