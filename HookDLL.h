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

#ifndef __HOOKDLL_H__
#define __HOOKDLL_H__

enum HookReturnCode
{
   HOOK_ERROR,
   HOOK_OK,
   HOOK_OK_REHOOK,
};

enum MenuItems
{
   VDM_TOGGLEONTOP = WM_USER+1,
   VDM_TOGGLEMINIMIZETOTRAY,
   VDM_TOGGLETRANSPARENCY,

   VDM_TOGGLEALLDESKTOPS,
   VDM_MOVEWINDOW,

   VDM_ACTIVATEWINDOW,
   VDM_RESTORE,
   VDM_MINIMIZE,
   VDM_MAXIMIZE,
   VDM_MAXIMIZEHEIGHT,
   VDM_MAXIMIZEWIDTH,
   VDM_CLOSE,
   VDM_KILL,

   VDM_PROPERTIES,

   VDM_MOVETODESK,
};

enum VirtualDimensionMessages
{
   WM_VD_HOOK_MENU_COMMAND = WM_APP + 100,
   WM_VD_PREPARE_HOOK_MENU,
   WM_VD_CHECK_MIN_TO_TRAY,
   WM_VD_MOUSEWARP,
	WM_VD_WNDSIZEMOVE,

   WM_VD_STARTONDESKTOP,      /* an application should start on the specified desktop. wParam = processId lParam = deskopIdx */
   WM_VD_SWITCHDESKTOP,       /* switch to some desktop. lParam = desktopIdx */

   WM_VD_LANGUAGE = WM_APP + 0x1000,   /* WM_VD_LANGUAGE to WM_VD_LANGUAGE + 0x1000 is reserved for all language messages) */
};

#endif /*__HOOKDLL_H__*/
