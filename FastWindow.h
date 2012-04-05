/* 
 * Fast Window - A fast and convenient message dispatching window procedure.
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
 
#ifndef __FASTWINDOW_H__
#define __FASTWINDOW_H__

#include <map>
#include <assert.h>

#define FASTWINDOW_NB_TIMER_MAX 50

class FastWindow
{
public:
   FastWindow();
   ~FastWindow(void);

   bool IsValid() const { return ::IsWindow(m_hWnd) ? true : false; }

   HWND Create( LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, 
                int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance);

   HWND Create( DWORD dwStyleEx, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, 
                int x, int y, int nWidth, int nHeight, 
                HWND hWndParent, HMENU hMenu, HINSTANCE hInstance);


   static ATOM RegisterClassEx(LPWNDCLASSEX lpwcex);
   static ATOM RegisterClass(LPWNDCLASS lpwc);

   template <class T> void SetMessageHandler(UINT message, T * object, LRESULT (T::*method)(HWND, UINT, WPARAM, LPARAM))
   {
      m_messageMap[message]((EventHandlerImp*)object, (EventHandlerImp::HandlerMethod)method); 
   }
   void UnSetMessageHandler(UINT message)
   {
      m_messageMap.erase(message);
   }

   template <class T> void SetCommandHandler(UINT message, T * object, LRESULT (T::*method)(HWND, UINT, WPARAM, LPARAM))
   {
      if (m_commandMap.empty())
         SetMessageHandler(WM_COMMAND, this, &FastWindow::CommandHandler);
      m_commandMap[message]((EventHandlerImp*)object, (EventHandlerImp::HandlerMethod)method); 
   }
   void UnSetCommandHandler(UINT message)
   {
      m_commandMap.erase(message);
      if (m_commandMap.empty())
         UnSetMessageHandler(WM_COMMAND);
   }

   template <class T> void SetSysCommandHandler(UINT message, T * object, LRESULT (T::*method)(HWND, UINT, WPARAM, LPARAM))
   {
      if (m_syscommandMap.empty())
         SetMessageHandler(WM_SYSCOMMAND, this, &FastWindow::SysCommandHandler);
      m_syscommandMap[message]((EventHandlerImp*)object, (EventHandlerImp::HandlerMethod)method); 
   }
   void UnSetSysCommandHandler(UINT message)
   {
      m_syscommandMap.erase(message);
      if (m_syscommandMap.empty())
         UnSetMessageHandler(WM_SYSCOMMAND);
   }

   template <class T> void SetNotifyHandler(UINT message, T * object, LRESULT (T::*method)(HWND, UINT, WPARAM, LPARAM))
   {
      if (m_notifyMap.empty())
         SetMessageHandler(WM_NOTIFY, this, &FastWindow::NotifyHandler);
      m_notifyMap[message]((EventHandlerImp*)object, (EventHandlerImp::HandlerMethod)method); 
   }
   void UnSetNotifyHandler(UINT message)
   {
      m_notifyMap.erase(message);
      if (m_notifyMap.empty())
         UnSetMessageHandler(WM_NOTIFY);
   }

   template <class T> UINT_PTR CreateTimer(T * object, LRESULT (T::*method)(HWND, UINT, WPARAM, LPARAM))
   {
      UINT_PTR timerId;
      if (m_timersMap.empty())
         SetMessageHandler(WM_TIMER, this, &FastWindow::TimersHandler);
      timerId = FindFreeTimerId();
      if (timerId)
         m_timersMap[timerId]((EventHandlerImp*)object, (EventHandlerImp::HandlerMethod)method);
      return timerId;
   }
   void DestroyTimer(UINT_PTR timerId)
   {
      assert(m_timersMap.find(timerId) != m_timersMap.end());
      KillTimer(timerId);
      m_timersMap.erase(timerId);
      if (m_timersMap.empty())
         UnSetMessageHandler(WM_TIMER);
   }
   void SetTimer(UINT_PTR timerId, UINT elapse)
   {
      assert(m_timersMap.find(timerId) != m_timersMap.end());
      ::SetTimer(*this, timerId, elapse, NULL);
   }
   void KillTimer(UINT_PTR timerId)
   {
      assert(m_timersMap.find(timerId) != m_timersMap.end());
      ::KillTimer(*this, timerId);
   }

   operator HWND()               { return m_hWnd; }

protected:
   class EventHandlerImp {
   public:
      typedef LRESULT (EventHandlerImp::*HandlerMethod)(HWND hWnd, UINT code, WPARAM wParam, LPARAM lParam);
   };

   class Handler {
   public:
      Handler() { return; }
      void operator()(EventHandlerImp * object, EventHandlerImp::HandlerMethod method)
      {  m_object = object; m_method = method; }

      LRESULT operator()(HWND hWnd, UINT code, WPARAM wParam, LPARAM lParam)
      {  return ((m_object)->*(m_method))(hWnd, code, wParam, lParam); }

   protected:
      EventHandlerImp * m_object;
      EventHandlerImp::HandlerMethod m_method;
   };

   typedef std::map<UINT, Handler> MessageMap;

   HWND m_hWnd;
   MessageMap m_messageMap;
   MessageMap m_commandMap;
   MessageMap m_syscommandMap;
   MessageMap m_notifyMap;
   MessageMap m_timersMap;
   UINT_PTR m_freeTimerId;

   LRESULT CommandHandler(HWND hWnd, UINT code, WPARAM wParam, LPARAM lParam);
   LRESULT SysCommandHandler(HWND hWnd, UINT code, WPARAM wParam, LPARAM lParam);
   LRESULT NotifyHandler(HWND hWnd, UINT code, WPARAM wParam, LPARAM lParam);
   LRESULT TimersHandler(HWND hWnd, UINT code, WPARAM wParam, LPARAM lParam);
   static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); 

   UINT_PTR FindFreeTimerId();
};

#endif /*__FASTWINDOW_H__*/
