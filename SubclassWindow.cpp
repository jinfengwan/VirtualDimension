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
#include "HookDLL.h"

typedef struct RemoteStartupArgs {
	HINSTANCE (WINAPI *fnLoadLibrary)(LPCTSTR);
   BOOL (WINAPI *fnFreeLibrary)(HINSTANCE);
   FARPROC (WINAPI *fnGetProcAddress)(HINSTANCE,LPCSTR);
	CHAR pbLibFile[MAX_PATH * sizeof(CHAR)];
   int fnIndex;
   HWND hWnd;
   int data;
} RemoteStartupArgs;

typedef struct RemoteCleanupArgs {
   FARPROC (WINAPI *fnGetProcAddress)(HINSTANCE,LPCSTR);
   int fnIndex;
   HWND hWnd;
   HINSTANCE hInstance;
} RemoteCleanupArgs;

// Calls to the stack-checking routine must be disabled.
#ifdef _MSC_VER
#pragma check_stack (off)
#pragma runtime_checks( "", off )
#endif /*_MSC_VER*/

static HINSTANCE WINAPI RemoteStartup (RemoteStartupArgs* args) 
{
	HINSTANCE hinstLib;
   DWORD (WINAPI *fnFunc)(HWND,int);
   DWORD res;

   //Try to load the library
  	hinstLib = args->fnLoadLibrary(args->pbLibFile);
   if (!hinstLib)
      return NULL;

   fnFunc = (DWORD (WINAPI *)(HWND,int))args->fnGetProcAddress(hinstLib, (LPCSTR)MAKEINTRESOURCE(args->fnIndex));

   res = fnFunc ? fnFunc(args->hWnd, args->data) : HOOK_ERROR;

   if (res == HOOK_ERROR)  //error
   {
      args->fnFreeLibrary(hinstLib);
      return NULL;
   }
   else  // OK
   {
      if (res == HOOK_OK_REHOOK)  //window was already hooked!
         //free the library to decrease its load count, and ensure it will be unloaded
         //when all windows are unhooked. This should NOT unload the library now.
         args->fnFreeLibrary(hinstLib);

      return hinstLib;
   }
}

static void AfterRemoteStartup(void)
{
}

static DWORD WINAPI RemoteCleanup(RemoteCleanupArgs* args) 
{
   DWORD (WINAPI *fnFunc)(HINSTANCE,HWND);

   fnFunc = (DWORD (WINAPI *)(HINSTANCE,HWND))args->fnGetProcAddress(args->hInstance, (LPCSTR)MAKEINTRESOURCE(args->fnIndex));

   return fnFunc &&
          fnFunc(args->hInstance, args->hWnd);
}

static void AfterRemoteCleanup(void)
{
}

#ifdef _MSC_VER
#pragma runtime_checks( "", restore )
#pragma check_stack 
#endif /*_MSC_VER*/

#define STACKPTR(Context)  (Context.Esp)

static PVOID AllocProcessMemory (HANDLE hProcess, DWORD dwNumBytes) {
	CONTEXT Context;
	DWORD dwThreadId, dwNumBytesXferred;
	HANDLE hThread;
	HINSTANCE hinstKrnl = GetModuleHandle(__TEXT("Kernel32"));
	PVOID pvMem;
	MEMORY_BASIC_INFORMATION mbi;

	hThread = CreateRemoteThread(hProcess, NULL,	dwNumBytes + sizeof(HANDLE),
         		(LPTHREAD_START_ROUTINE)GetProcAddress(hinstKrnl, "ExitThread"), 
               0,	CREATE_SUSPENDED,	&dwThreadId);
	if (hThread == NULL)
		return NULL;

	Context.ContextFlags = CONTEXT_CONTROL;
	if (!GetThreadContext(hThread, &Context))
   {
      ResumeThread(hThread);  //let the thread run and vanish
      return NULL;
   }

	if (sizeof(mbi) != VirtualQueryEx(hProcess, (PDWORD)STACKPTR(Context) - 1, &mbi, sizeof(mbi)))
   {
      ResumeThread(hThread);  //let the thread run and vanish
      return NULL;
   }

	pvMem = (PVOID) mbi.BaseAddress;
	if (!WriteProcessMemory(hProcess, pvMem, &hThread, sizeof(hThread), &dwNumBytesXferred)) 
   {
      ResumeThread(hThread);  //let the thread run and vanish
      return NULL;
   }

   pvMem = (PVOID) ((PHANDLE) pvMem + 1);

   return pvMem;
}

static BOOL FreeProcessMemory (HANDLE hProcess, PVOID pvMem)
{
	BOOL res;
	HANDLE hThread;
	DWORD dwNumBytesXferred;

	// Get the handle of the remote thread from the block of memory.
	pvMem = (PVOID) ((PHANDLE) pvMem - 1);

   res = ReadProcessMemory(hProcess, pvMem, &hThread, sizeof(hThread), &dwNumBytesXferred);
	if (res)
   {
		if (ResumeThread(hThread) == 0xffffffff)
			res = FALSE;
		CloseHandle(hThread);
	}

	return res;
}

HINSTANCE HookWindow(HWND hWnd, DWORD dwProcessId, int data)
{
   const int cbCodeSize = ((LPBYTE)AfterRemoteStartup - (LPBYTE)RemoteStartup);
	const DWORD cbMemSize = cbCodeSize + sizeof(RemoteStartupArgs) + 3;
   HANDLE hProcess;
	HINSTANCE hinstKrnl = GetModuleHandle("Kernel32");
	PDWORD pdwCodeRemote = NULL;
   RemoteStartupArgs args;
   RemoteStartupArgs * pArgsRemote;
	DWORD dwNumBytesXferred = 0;
	HANDLE hThread = NULL;
   DWORD dwOldProtect;
   HINSTANCE res;

   hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
   if (!hProcess)
      return NULL;

   //Allocate some memory for our function and its argument
	pdwCodeRemote = (PDWORD) AllocProcessMemory(hProcess, cbMemSize);
	if (!pdwCodeRemote)
   {
      CloseHandle(hProcess);
      return NULL;
   }

	// Change the page protection of the allocated memory to executable, read, and write.
	if (!VirtualProtectEx(hProcess, pdwCodeRemote, cbMemSize, PAGE_EXECUTE_READWRITE, &dwOldProtect))
   {
      FreeProcessMemory(hProcess, pdwCodeRemote);
      CloseHandle(hProcess);
      return NULL;
   }

	// Write a copy of the function to the remote process.
   if (!WriteProcessMemory(hProcess, pdwCodeRemote, (LPVOID)RemoteStartup, cbCodeSize, &dwNumBytesXferred))
   {
      FreeProcessMemory(hProcess, pdwCodeRemote);
      CloseHandle(hProcess);
      return NULL;
   }

   // Initialize the arguments
   args.fnLoadLibrary = (HINSTANCE (WINAPI *)(LPCTSTR))
                          GetProcAddress(hinstKrnl, "LoadLibraryA");
   args.fnFreeLibrary = (BOOL (WINAPI *)(HINSTANCE))
                          GetProcAddress(hinstKrnl, "FreeLibrary");
   args.fnGetProcAddress = (FARPROC (WINAPI *)(HINSTANCE,LPCSTR))GetProcAddress(hinstKrnl, "GetProcAddress");
   args.hWnd = hWnd;
   args.data = data;
   args.fnIndex = 0x1;
   DWORD result = SearchPathA(NULL, "HookDLL.dll", NULL, MAX_PATH, args.pbLibFile, NULL);
   if ((result < 0) || (result > MAX_PATH))
   {
      FreeProcessMemory(hProcess, pdwCodeRemote);
      CloseHandle(hProcess);
      return NULL;
   }

	// Write a copy of the arguments to the remote process (the structure MUST start on a 32-bit bourdary)
   pArgsRemote = (RemoteStartupArgs*)(pdwCodeRemote + ((cbCodeSize + 4) & ~3));
   if (!WriteProcessMemory(hProcess, pArgsRemote,	&args, sizeof(args), &dwNumBytesXferred))
   {
      FreeProcessMemory(hProcess, pdwCodeRemote);
      CloseHandle(hProcess);
      return NULL;
   }

   //Run the remote function
	hThread = CreateRemoteThread(hProcess, NULL, 10000, (LPTHREAD_START_ROUTINE)pdwCodeRemote, pArgsRemote, 0, NULL);
	if (hThread == NULL)
   {
      FreeProcessMemory(hProcess, pdwCodeRemote);
      CloseHandle(hProcess);
      return NULL;
   }

   //Wait till it returns, then get the result
	WaitForSingleObject(hThread, INFINITE);
   GetExitCodeThread(hThread, (PDWORD) &res);
	
   //Do some cleanup
   CloseHandle(hThread);
   FreeProcessMemory(hProcess, pdwCodeRemote);
   CloseHandle(hProcess);

   return res;
}

bool UnHookWindow(HINSTANCE hInstance, DWORD dwProcessId, HWND hWnd)
{
   const int cbCodeSize = ((LPBYTE)AfterRemoteCleanup - (LPBYTE)RemoteCleanup);
	const DWORD cbMemSize = cbCodeSize + sizeof(RemoteCleanupArgs) + 3;
   HANDLE hProcess;
	HINSTANCE hinstKrnl = GetModuleHandle("Kernel32");
	PDWORD pdwCodeRemote = NULL;
   RemoteCleanupArgs args;
   RemoteCleanupArgs * pArgsRemote;
	DWORD dwNumBytesXferred = 0;
	HANDLE hThread = NULL;
	BOOL res;
   DWORD dwOldProtect;

   hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
   if (!hProcess)
      return false;

   //Allocate some memory for our function and its argument
	pdwCodeRemote = (PDWORD) AllocProcessMemory(hProcess, cbMemSize);
	if (!pdwCodeRemote)
   {
      CloseHandle(hProcess);
      return false;
   }

	// Change the page protection of the allocated memory to executable, read, and write.
	if (!VirtualProtectEx(hProcess, pdwCodeRemote, cbMemSize, PAGE_EXECUTE_READWRITE, &dwOldProtect))
   {
      FreeProcessMemory(hProcess, pdwCodeRemote);
      CloseHandle(hProcess);
      return true;
   }

	// Write a copy of the function to the remote process.
   if (!WriteProcessMemory(hProcess, pdwCodeRemote, (LPVOID)RemoteCleanup, cbCodeSize, &dwNumBytesXferred))
   {
      FreeProcessMemory(hProcess, pdwCodeRemote);
      CloseHandle(hProcess);
      return true;
   }

   // Initialize the arguments
   args.fnGetProcAddress = (FARPROC (WINAPI *)(HINSTANCE,LPCSTR))GetProcAddress(hinstKrnl, "GetProcAddress");
   args.hWnd = hWnd;
   args.fnIndex = 0x2;
   args.hInstance = hInstance;

	// Write a copy of the arguments to the remote process (the structure MUST start on a 32-bit bourdary)
   pArgsRemote = (RemoteCleanupArgs*)(pdwCodeRemote + ((cbCodeSize + 4) & ~3));
   if (!WriteProcessMemory(hProcess, pArgsRemote,	&args, sizeof(args), &dwNumBytesXferred))
   {
      FreeProcessMemory(hProcess, pdwCodeRemote);
      CloseHandle(hProcess);
      return true;
   }

   //Run the remote function
   DWORD dwThreadId;
	hThread = CreateRemoteThread(hProcess, NULL, 10000, (LPTHREAD_START_ROUTINE)pdwCodeRemote, pArgsRemote, 0, &dwThreadId);
	if (hThread == NULL)
   {
      FreeProcessMemory(hProcess, pdwCodeRemote);
      CloseHandle(hProcess);
      return true;
   }

   //Wait till it returns, then get the result
	WaitForSingleObject(hThread, INFINITE);
   GetExitCodeThread(hThread, (PDWORD) &res);
	
   //Do some cleanup
   CloseHandle(hThread);
   FreeProcessMemory(hProcess, pdwCodeRemote);
   CloseHandle(hProcess);

   return (res ? true : false);
}
