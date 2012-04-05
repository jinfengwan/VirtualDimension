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
 
#ifndef _BALLOONNOTIF_H_
#define _BALLOONNOTIF_H_

class BalloonNotification
{
 public:
   BalloonNotification(void);
   ~BalloonNotification(void);

   typedef HWND Message;
   typedef void (*ClickCb)(Message tip, int data);   ///< Callback called when a message is clicked.

   /** Add a new message to the notification area. 
    * @param icon Icon to display. can be one of the NIIN_* values, or an HICON (works 
    * on WindowsXP SP2 only). Only displayed if a title is provided as well.
    * @param timeout Timeout in millisecond before automatically disappearing. 0 to disable.
    */
   Message Add(LPCTSTR text, LPCTSTR title=NULL, int icon=0, ClickCb cb=NULL, int data=0, DWORD timeout=0);
   void Remove(Message msg);               ///< Remove a message from the notification area.

 protected:
   WNDPROC m_tooltipWndProc;  ///< Original tooltip window proc
   int m_tooltipWndExtra;     ///< Original tooltip window data byte count

   LPCTSTR m_tooltipClass;    ///< Window class to use for the balloon tooltips

   static LRESULT CALLBACK MyTooltipWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
};

extern BalloonNotification msgManager;

#endif /*_BALLOONNOTIF_H_*/
