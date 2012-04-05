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

#ifndef __TASKPOOL_H__
#define __TASKPOOL_H__

#include <list>

using namespace std;

class TaskPool
{
public:
   typedef void (JobProc)(void * arg);

   TaskPool(int minThread = 0, int maxThread = 100000);
   ~TaskPool(void);

   void AddThread();
   void DelThread();
   void SetThreadCount(LONG count);
   LONG GetThreadCount() const { return m_nThreadCount; }

   void QueueJob(JobProc * fun, void * arg, HANDLE * event=NULL);
   bool DeQueueJob(JobProc * fun, void * arg);
   bool UpdateJob(JobProc * oldFun, void * oldArg, JobProc * newFun, void * newArg, HANDLE * event=NULL);

protected:
   DWORD task();
   static DWORD WINAPI taskProxy(LPVOID lpParameter);

   HANDLE m_hPendingWindowsSem;
   HANDLE m_hStopThread;

   LONG m_nThreadCount;
   LONG m_nMinThreadCount;
   LONG m_nMaxThreadCount;

   //Time in ms after which an inactive thread is automatically destroyed.
   //Set to INFINITE in order to have threads to last forever.
   DWORD m_dwThreadTimeout;

   typedef struct JobInfo {
      JobProc * fun;
      void * arg;
      HANDLE event;
   } JobInfo;
   list<JobInfo> m_jobsQueue;
   HANDLE m_hQueueMutex;
};

#endif /*__TASKPOOL_H__*/
