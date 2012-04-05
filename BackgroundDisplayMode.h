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

#ifndef __BACKGROUNDDISPLAYMODE_H__
#define __BACKGROUNDDISPLAYMODE_H__

class BackgroundDisplayMode
{
public:
   virtual ~BackgroundDisplayMode() { return; }

   virtual void BeginPainting(HDC hdc);
   virtual void PaintDesktop(HDC hdc, LPRECT rect, bool active);
   virtual void EndPainting(HDC hdc);

   virtual void ReSize(int width, int height);

   virtual bool ChooseOptions(HWND hWnd);
};

class PictureBackgroundDisplayMode: public BackgroundDisplayMode
{
public:
   PictureBackgroundDisplayMode();
   virtual ~PictureBackgroundDisplayMode();

   virtual void BeginPainting(HDC hdc);
   virtual void PaintDesktop(HDC hdc, LPRECT rect, bool active);
   virtual void EndPainting(HDC hdc);

   virtual void ReSize(int width, int height);

   virtual bool ChooseOptions(HWND hWnd);

protected:
   void UpdatePictureObjects();

   TCHAR m_bkgrndPictureFile[MAX_PATH];

   HBITMAP m_selDeskBkPicture;
   HBITMAP m_deskBkPicture;

   HDC m_picDC;

   int m_height;
   int m_width;
};

class PlainColorBackgroundDisplayMode: public BackgroundDisplayMode
{
public:
   PlainColorBackgroundDisplayMode();
   virtual ~PlainColorBackgroundDisplayMode();

   virtual void PaintDesktop(HDC hdc, LPRECT rect, bool active);

   virtual bool ChooseOptions(HWND hWnd);

protected:
   void UpdateBrushObjects();

   COLORREF m_bkgrndColor;

   HBRUSH m_selDeskBkBrush;
   HBRUSH m_deskBkBrush;
};

#endif /*__BACKGROUNDDISPLAYMODE_H__*/
