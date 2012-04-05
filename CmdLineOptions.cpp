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

#include "stdafx.h"
#include "shellapi.h"
#include "CmdLine.h"
#include "VirtualDimension.h"
#include "HookDLL.h"


class CommandLineStartApp : public CommandLineOption {
public:
   CommandLineStartApp(char opcode, UINT resid): CommandLineOption(opcode, resid, required_argument)   {}
   virtual void ParseOption(LPCTSTR arg);
};

class CommandLineSwitchDesktop : public CommandLineOption {
public:
   CommandLineSwitchDesktop(char opcode, UINT resid): CommandLineOption(opcode, resid, required_argument)   {}
   virtual void ParseOption(LPCTSTR arg);
};

//CommandLineInt g_cmdLineTransp('t', 0, 0, 192, CommandLineOption::optional_argument);	//start the application with transparency enabled
//CommandLineFlag g_cmdLineMinToTray('m', 0);												//start the application with MinToTray flag
CommandLineInt g_cmdLineDesktop('d', 0, -1, -1, CommandLineOption::optional_argument);	//Desktop on which to start the application. default is current desktop
CommandLineStartApp g_cmdLineStartApp('x', 0);											//Start an application
CommandLineSwitchDesktop g_cmdLineSwitchDesk('s', 0);									//Switch current desktop

void CommandLineStartApp::ParseOption(LPCTSTR arg)
{
	//start the specified application !!!
	SHELLEXECUTEINFO info;
	info.cbSize = sizeof(info);
	info.fMask = SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
	info.lpFile = arg;
	info.lpDirectory = NULL;
	info.lpParameters = NULL;
	info.lpVerb = NULL;
	info.nShow = SW_SHOW;
	if (ShellExecuteEx(&info) && (int)info.hInstApp > 32 && info.hProcess)
	{
		HWND hWnd = vdWindow.FindWindow();
		Sleep(500);	//give some time for the window to start...
		if (hWnd && g_cmdLineDesktop != -1)
			//would look better to execute the window 'hidden' (minized...), then show it on the right desktop
			PostMessage(hWnd, WM_VD_STARTONDESKTOP, (WPARAM)GetProcessId(info.hProcess), (LPARAM)g_cmdLineDesktop);
		CloseHandle(info.hProcess);
	}
}

void CommandLineSwitchDesktop::ParseOption(LPCTSTR arg)
{
	//switch to the specified desktop
	int desk = strtol(arg, NULL, 0);
	HWND hWnd = vdWindow.FindWindow();
	if (hWnd)
	PostMessage(hWnd, WM_VD_SWITCHDESKTOP, 0, desk);
}

