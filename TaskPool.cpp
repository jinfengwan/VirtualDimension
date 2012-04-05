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
#include "TaskPool.h"
#include "Window.h"

TaskPool::TaskPool(int minThread, int maxThread)
{
   m_hPendingWindowsSem = CreateSemaphore(NULL, 0, 50, NULL);
   m_hStopThread = CreateEvent(NULL, FALSE, FALSE, NULL);
   m_hQueueMutex = CreateMutex(NULL, FALSE, NULL);

   m_nThreadCount = 0;

   m_dwThreadTimeout = 1000*600;   // 10 minutes
   m_nMinThreadCount = minThread;
   m_nMaxThreadCount = maxThread;

   for(int i=0; i<m_nMinThreadCount; i++)
      AddThread();
}

TaskPool::~TaskPool(void)
{
   m_nMinThreadCount = 0;  //Ensure all threads will get distroyed
   for(int i = 0; i < m_nThreadCount; i++)
      DelThread();
}

void TaskPool::AddThread()
{
   if (m_nThreadCount < m_nMaxThreadCount)
      CreateThread(NULL, 0, &taskProxy, this, 0, NULL);
}

void TaskPool::DelThread()
{
   if (m_nThreadCount <= 0)
      return;

   SetEvent(m_hStopThread);
}

void TaskPool::SetThreadCount(LONG count)
{
   LONG delta = count - m_nThreadCount;

   if (delta > 0)
   {
      for(LONG i=0; i<delta; i++)
         AddThread();
   }
   else if (delta < 0)
   {
      for(LONG i=delta; i<0; i++)
         DelThread();
   }
}

void TaskPool::QueueJob(JobProc * fun, void * arg, HANDLE * event)
{
   static JobInfo info;
   int length;

   //Get access to the queue
   WaitForSingleObject(m_hQueueMutex, INFINITE);

   //Build the job info
   info.fun = fun;
   info.arg = arg;
   if (event)
      *event = info.event = CreateEvent(NULL, FALSE, FALSE, NULL);
   else
      info.event = NULL;

   //Add the job
   m_jobsQueue.push_back(info);

   //Get the length of the queue
   length = m_jobsQueue.size();

   //Release access to the queue
   ReleaseMutex(m_hQueueMutex);

   //Let the new job be processed
   ReleaseSemaphore(m_hPendingWindowsSem, 1, NULL);

   //Increase the number of thread, if needed
   if ((length<<1) > m_nThreadCount)
      AddThread();
}

bool TaskPool::DeQueueJob(JobProc * fun, void * arg)
{
   //Get access to the queue
   WaitForSingleObject(m_hQueueMutex, INFINITE);

   list<JobInfo>::iterator it;
   for(it = m_jobsQueue.begin(); it != m_jobsQueue.end(); it++)
   {
      if ((it->arg == arg) && (it->fun == fun))
      {
         m_jobsQueue.erase(it);
            
         ReleaseMutex(m_hQueueMutex);
         return true;
      }
   }

   //Release access to the queue
   ReleaseMutex(m_hQueueMutex);

   return false;
}

bool TaskPool::UpdateJob(JobProc * oldFun, void * oldArg, JobProc * newFun, void * newArg, HANDLE * event)
{
   //Get access to the queue
   WaitForSingleObject(m_hQueueMutex, INFINITE);

   list<JobInfo>::iterator it;
   for(it = m_jobsQueue.begin(); it != m_jobsQueue.end(); it++)
   {
      if ((it->arg == oldArg) && (it->fun == oldFun))
      {
         it->arg = newArg;
         it->fun = newFun;
         if (event && it->event)
            *event = it->event;
         else if (event)
            *event = it->event = CreateEvent(NULL, FALSE, FALSE, NULL);
         else if (it->event)
         {
            CloseHandle(it->event);
            it->event = NULL;
         }

         ReleaseMutex(m_hQueueMutex);
         return true;
      }
   }

   //Release access to the queue
   ReleaseMutex(m_hQueueMutex);

   return false;
}

DWORD TaskPool::task()
{
   HANDLE objects[2] = { m_hStopThread, m_hPendingWindowsSem };

   InterlockedIncrement(&m_nThreadCount);
   do
      while(WaitForMultipleObjects(2, objects, FALSE, m_dwThreadTimeout) == WAIT_OBJECT_0+1)
      {
         JobProc * fun;
         void * arg;
         HANDLE event;

         //Get access to the queue
         WaitForSingleObject(m_hQueueMutex, INFINITE);

         //Remove a job from the queue
         if (!m_jobsQueue.empty())
         {
            JobInfo& info = m_jobsQueue.front();

            fun = info.fun;
            arg = info.arg;
            event = info.event;

            m_jobsQueue.pop_front();
         }
         else
         {
            fun = NULL;
            arg = NULL;
            event = NULL;
         }

         //Release access to the queue
         ReleaseMutex(m_hQueueMutex);

         //Do the job !
         if (fun)
            fun(arg);
            
         //Notify job completion
         if (event)
         {
         	PulseEvent(event);
         	CloseHandle(event);
         }
      }
   while(m_nThreadCount <= m_nMinThreadCount);  //Ensure a minimum number of threads is running
   InterlockedDecrement(&m_nThreadCount);

   return 0;
}

DWORD WINAPI TaskPool::taskProxy(LPVOID lpParameter)
{
   return ((TaskPool*)lpParameter)->task();
}
