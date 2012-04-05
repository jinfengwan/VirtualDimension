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
#include "settings.h"

const char Settings::regKeyName[] = "Software\\Typz Software\\Virtual Dimension\\";

static const RECT DefaultWindowPosition = {10, 10, 110, 110};
static const LOGFONT DefaultPreviewWindowFont = {-12/*height*/,0,0,0,FW_BOLD/*weight*/,FALSE/*italic*/,0,0,0,0,0,0,0,"Arial"/*fontname*/};
static const LOGFONT DefaultOSDFont = {-29/*height*/,0,0,0,FW_BOLD/*weight*/,TRUE/*italic*/,0,0,0,0,0,0,0,"Arial"/*fontname*/};
static const POINT DefaultOSDPosition = {50,50};

DEFINE_SETTING(Settings, WindowPosition, RECT, &DefaultWindowPosition);
DEFINE_SETTING(Settings, DockedBorders, int, 0);
DEFINE_SETTING(Settings, ColumnNumber, unsigned long, 2);
DEFINE_SETTING(Settings, LockPreviewWindow, bool, false);
DEFINE_SETTING(Settings, ShowWindow, bool, true);
DEFINE_SETTING(Settings, HasTrayIcon, bool, true);
DEFINE_SETTING(Settings, AlwaysOnTop, bool, false);
DEFINE_SETTING(Settings, TransparencyLevel, unsigned char, 0xff);
DEFINE_SETTING(Settings, HasCaption, bool, true);
DEFINE_SETTING(Settings, SnapSize, int, 15);
DEFINE_SETTING(Settings, AutoHideDelay, int, 0);
DEFINE_SETTING(Settings, EnableToolTips, bool, true);
DEFINE_SETTING(Settings, ConfirmKilling, bool, true);
DEFINE_SETTING(Settings, AutoSaveWindowSettings, bool, false);
DEFINE_SETTING(Settings, CloseToTray, bool, false);
DEFINE_SETTING(Settings, AutoSwitchDesktop, bool, true);
DEFINE_SETTING(Settings, AllWindowsInTaskList, bool, false);
DEFINE_SETTING(Settings, IntegrateWithShell, bool, true);
DEFINE_SETTING(Settings, SwitchToNextDesktopHotkey, int, 0);
DEFINE_SETTING(Settings, SwitchToPreviousDesktopHotkey, int, 0);
DEFINE_SETTING(Settings, SwitchToTopDesktopHotkey, int, 0);
DEFINE_SETTING(Settings, SwitchToBottomDesktopHotkey, int, 0);
DEFINE_SETTING(Settings, SwitchToLeftDesktopHotkey, int, 0);
DEFINE_SETTING(Settings, SwitchToRightDesktopHotkey, int, 0);
DEFINE_SETTING(Settings, MoveWindowToNextDesktopHotkey, int, 0);
DEFINE_SETTING(Settings, MoveWindowToPreviousDesktopHotkey, int, 0);
DEFINE_SETTING(Settings, MoveWindowToDesktopHotkey, int, 0);
DEFINE_SETTING(Settings, MaximizeHeightHotkey, int, 0);
DEFINE_SETTING(Settings, MaximizeWidthHotkey, int, 0);
DEFINE_SETTING(Settings, AlwaysOnTopHotkey, int, 0);
DEFINE_SETTING(Settings, TransparencyHotkey, int, 0);
DEFINE_SETTING(Settings, TogglePreviewWindowHotkey, int, 0);
DEFINE_SETTING(Settings, DisplayMode, int, 0);
DEFINE_SETTING(Settings, BackgroundColor, COLORREF, RGB(0xc0,0xc0,0xc0));
DEFINE_SETTING(Settings, BackgroundPicture, LPTSTR, "");
DEFINE_SETTING(Settings, DesktopNameOSD, bool, false);
DEFINE_SETTING(Settings, PreviewWindowFont, LOGFONT, &DefaultPreviewWindowFont);
DEFINE_SETTING(Settings, PreviewWindowFontColor, COLORREF, RGB(0,0,0));
DEFINE_SETTING(Settings, OSDTimeout, int, 2000);
DEFINE_SETTING(Settings, OSDFont, LOGFONT, DefaultOSDFont);
DEFINE_SETTING(Settings, OSDFgColor, COLORREF, RGB(0,0,0));
DEFINE_SETTING(Settings, OSDBgColor, COLORREF, RGB(255,255,255));
DEFINE_SETTING(Settings, OSDPosition, POINT, &DefaultOSDPosition);
DEFINE_SETTING(Settings, OSDTransparencyLevel, unsigned char, 200);
DEFINE_SETTING(Settings, OSDHasBackground, bool, true);
DEFINE_SETTING(Settings, OSDIsTransparent, bool, true);
DEFINE_SETTING(Settings, WarpEnable, bool, false);
DEFINE_SETTING(Settings, WarpSensibility, LONG, 3);
DEFINE_SETTING(Settings, WarpMinDuration, DWORD, 500);
DEFINE_SETTING(Settings, WarpRewarpDelay, DWORD, 3000);
DEFINE_SETTING(Settings, WarpRequiredVKey, int, 0);
DEFINE_SETTING(Settings, WarpInvertMousePos, bool, true);
DEFINE_SETTING(Settings, DefaultHidingMethod, int, 0);
DEFINE_SETTING(Settings, LanguageCode, int, 0);

Settings::Settings(void): RegistryGroup(regKeyName)
{
}

DWORD Settings::LoadDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD defVal)
{
   DWORD size;
   DWORD val;

   if ( (!keyOpened) ||
        (RegQueryValueEx(regKey, entry, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(val)) ||
        (RegQueryValueEx(regKey, entry, NULL, NULL, (LPBYTE)&val, &size) != ERROR_SUCCESS) )
   {
      // Cannot load the value from registry --> set default value
      val = defVal;
   }

   return val;
}

void Settings::SaveDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD value)
{
   if (keyOpened)
      RegSetValueEx(regKey, entry, 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
}

bool Settings::LoadBinary(HKEY regKey, bool keyOpened, const char * entry, LPBYTE buffer, DWORD length)
{
   DWORD size;

   return (keyOpened) &&
          (RegQueryValueEx(regKey, entry, NULL, NULL, NULL, &size) == ERROR_SUCCESS) &&
          (size == length) &&
          (RegQueryValueEx(regKey, entry, NULL, NULL, buffer, &size) == ERROR_SUCCESS);
}

void Settings::SaveBinary(HKEY regKey, bool keyOpened, const char * entry, LPBYTE buffer, DWORD length)
{
   if (keyOpened)
      RegSetValueEx(regKey, entry, 0, REG_BINARY, buffer, length);
}

const char Settings::regKeyWindowsStartup[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Run\\";
const char Settings::regValStartWithWindows[] = "Virtual Dimension";

bool Settings::LoadStartWithWindows()
{
   HKEY regKey;
   if ((RegOpenKeyEx(HKEY_CURRENT_USER, regKeyWindowsStartup, 0, KEY_READ, &regKey) == ERROR_SUCCESS) &&
       (RegQueryValueEx(regKey, regValStartWithWindows, NULL, NULL, NULL, NULL) == ERROR_SUCCESS))
   {
      RegCloseKey(regKey);
      return true;
   }
   else
      return false;
}

void Settings::SaveStartWithWindows(bool start)
{
   HKEY regKey;
   if (RegOpenKeyEx(HKEY_CURRENT_USER, regKeyWindowsStartup, 0, KEY_WRITE, &regKey) == ERROR_SUCCESS)
   {
      if (start)
      {
         TCHAR buffer[256];
         GetModuleFileName(NULL, buffer, sizeof(buffer)/sizeof(TCHAR));
         RegSetValueEx(regKey, regValStartWithWindows, 0, REG_SZ, (LPBYTE)buffer, sizeof(buffer)/sizeof(TCHAR));
      }
      else
         RegDeleteValue(regKey, regValStartWithWindows);
      RegCloseKey(regKey);
   }
}

const char Settings::regSubKeyDisableShellIntegration[] = "DisableShellIntegration";
const char Settings::regSubKeyHidingMethods[] = "HidingMethodsTweaks";

bool Settings::LoadDisableShellIntegration(const char * windowclass)
{
   HKEY regKey=NULL;
   bool res;

   res = m_opened &&
         RegOpenKeyEx(m_regKey, regSubKeyDisableShellIntegration, 0, KEY_READ, &regKey) == ERROR_SUCCESS &&
         RegQueryValueEx(regKey, windowclass, NULL, NULL, NULL, NULL) == ERROR_SUCCESS;

   if (regKey!=NULL)
      RegCloseKey(regKey);

   return res;
}

void Settings::SaveDisableShellIntegration(const char * windowclass, bool enable)
{
   HKEY regKey=NULL;

   if (m_opened &&
       RegOpenKeyEx(m_regKey, regSubKeyDisableShellIntegration, 0, KEY_READ, &regKey) == ERROR_SUCCESS)
   {
      DWORD value = 0;

      if (enable)
         RegDeleteValue(regKey, windowclass);
      else
         RegSetValueEx(regKey, windowclass, 0, REG_DWORD, (BYTE*)&value, sizeof(value));

      RegCloseKey(regKey);
   }
}

int Settings::LoadHidingMethod(const char * windowclass)
{
   HKEY regKey=NULL;
   DWORD val;
   DWORD size = sizeof(val);
   int res;

   res = (m_opened &&
          RegOpenKeyEx(m_regKey, regSubKeyHidingMethods, 0, KEY_READ, &regKey) == ERROR_SUCCESS &&
          RegQueryValueEx(regKey, windowclass, NULL, NULL, (BYTE*)&val, &size) == ERROR_SUCCESS) ? val : LoadSetting(DefaultHidingMethod);

   if (regKey!=NULL)
      RegCloseKey(regKey);

   return res;
}

void Settings::SaveHidingMethod(const char * windowclass, int method)
{
   HKEY regKey=NULL;

   if (m_opened &&
       RegOpenKeyEx(m_regKey, regSubKeyHidingMethods, 0, KEY_READ, &regKey) == ERROR_SUCCESS)
   {
      if (method == 0)
         RegDeleteValue(regKey, windowclass);
      else
         RegSetValueEx(regKey, windowclass, 0, REG_DWORD, (BYTE*)&method, sizeof(method));

      RegCloseKey(regKey);
   }
}

const char Settings::Desktop::regKeyDesktops[] = "Desktops";

DEFINE_SETTING(Settings::Desktop, DeskIndex, int, 0);
DEFINE_SETTING(Settings::Desktop, DeskWallpaper, LPTSTR, "");
DEFINE_SETTING(Settings::Desktop, DeskHotkey, int, 0);
DEFINE_SETTING(Settings::Desktop, BackgroundColor, COLORREF, GetSysColor(COLOR_DESKTOP));

Settings::SubkeyList::SubkeyList(Settings * settings, const char regKey[]): m_group(*settings, regKey)
{
   *m_name = 0;
}

Settings::SubkeyList::SubkeyList(Settings * settings, const char regKey[], int index): m_group(*settings, regKey)
{
   Open(index);
}

Settings::SubkeyList::SubkeyList(Settings * settings, const char regKey[], char * name, bool create): m_group(*settings, regKey)
{
   Open(name, create);
}

bool Settings::SubkeyList::Open(const char * name, bool create)
{
	strcpy(m_name, name);
	return Config::RegistryGroup::Open(m_group, name, create);
}

bool Settings::SubkeyList::Open(int index)
{
   DWORD length;
   HRESULT result;

   if (m_opened)
      Close();

   length = sizeof(m_name);
   m_opened =
      (m_group.IsOpened()) &&
      (((result = RegEnumKeyEx(m_group, index, m_name, &length, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS) || (result == ERROR_MORE_DATA)) &&
      (RegOpenKeyEx(m_group, m_name, 0, KEY_ALL_ACCESS, &m_regKey) == ERROR_SUCCESS);

   return m_opened;
}

bool Settings::SubkeyList::IsValid()
{
   return m_opened;
}

void Settings::SubkeyList::Destroy()
{
   if (m_opened)
      Close();
   else
      return;

   if (m_group)
      RegDeleteKey(m_group, m_name);
}

char * Settings::SubkeyList::GetName(char * buffer, unsigned int length)
{
   if (m_opened && (buffer != NULL))
      strncpy(buffer, m_name, length);

   return buffer;
}

bool Settings::SubkeyList::Rename(char * buffer)
{
   HKEY newKey;
   DWORD index, max_index;
   LPSTR value;
   LPBYTE data;
   DWORD value_len, type, data_len;

   if (!m_opened || (strncmp(buffer, m_name, MAX_NAME_LENGTH) == 0))
      return m_opened;

   if ( (!m_group) ||
        (RegCreateKeyEx(m_group, buffer, 0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WRITE, NULL, &newKey, NULL)  != ERROR_SUCCESS) )
      return false;

   RegQueryInfoKey(newKey, NULL, NULL, NULL, NULL, NULL, NULL,
      &max_index, &value_len, &data_len, NULL, NULL);

   value = new TCHAR[value_len+1];  //Returned length does not include the trailing NULL character
   data = new BYTE[data_len];

   for(index = 0; index < max_index; index ++)
   {
      RegEnumValue(m_regKey, index, value, &value_len, 0, &type, data, &data_len);
      RegSetValueEx(newKey, value, 0, type, data, data_len);
   }

   Destroy();
   m_regKey = newKey;
   m_opened = true;
   strncpy(m_name, buffer, MAX_NAME_LENGTH);

   return true;
}

const char Settings::Window::regKeyWindows[] = "Windows";

static const RECT DefaultWindowAutoPosition = { 0, 200, 0, 300 };

DEFINE_SETTING(Settings::Window, AlwaysOnTop, bool, false);
DEFINE_SETTING(Settings::Window, OnAllDesktops, bool, false);
DEFINE_SETTING(Settings::Window, MinimizeToTray, bool, false);
DEFINE_SETTING(Settings::Window, TransparencyLevel, unsigned char, 0xc0);
DEFINE_SETTING(Settings::Window, EnableTransparency, bool, false);
DEFINE_SETTING(Settings::Window, AutoSaveSettings, bool, false);
DEFINE_SETTING(Settings::Window, WindowPosition, RECT, DefaultWindowAutoPosition);
DEFINE_SETTING(Settings::Window, AutoSetSize, bool, false);
DEFINE_SETTING(Settings::Window, AutoSetPos, bool, false);
DEFINE_SETTING(Settings::Window, AutoSetDesk, bool, false);
DEFINE_SETTING(Settings::Window, DesktopIndex, int, -1);
