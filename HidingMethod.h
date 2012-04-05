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

#ifndef __HIDINGMETHOD_H__
#define __HIDINGMETHOD_H__

class Window;

class HidingMethod
{
public:
   inline void SetWindowData(Window * wnd, int data);
   inline int GetWindowData(const Window * wnd);

   virtual void Attach(Window * /*wnd*/)            { return; }
   virtual void Detach(Window * /*wnd*/)            { return; }
   virtual void Show(Window * wnd) = 0;
   virtual void Hide(Window * wnd) = 0;
   virtual bool CheckCreated(Window * /*wnd*/)      { return true; }
   virtual bool CheckDestroyed(Window * /*wnd*/)    { return true; }
   virtual bool CheckSwitching(const Window * /*wnd*/)    { return false; }
};

class HidingMethodHide: public HidingMethod
{
public:
   virtual void Attach(Window * wnd);
   virtual void Show(Window * wnd);
   virtual void Hide(Window * wnd);
   virtual bool CheckCreated(Window * wnd);
   virtual bool CheckDestroyed(Window * wnd);
   virtual bool CheckSwitching(const Window * wnd);

protected:
   enum States {
      SHOWN,
      SHOWING,
      SHOWING_TO_HIDE,
      HIDDEN,
      HIDING,
      HIDING_TO_SHOW
   };
};

class HidingMethodMinimize: public HidingMethod
{
public:
   virtual void Attach(Window * wnd);
   virtual void Show(Window * wnd);
   virtual void Hide(Window * wnd);
};

class HidingMethodMove: public HidingMethod
{
public:
   virtual void Attach(Window * wnd);
   virtual void Show(Window * wnd);
   virtual void Hide(Window * wnd);
};

#endif /*__HIDINGMETHOD_H__*/
