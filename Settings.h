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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <assert.h>
#include "Config.h"

#define MAX_NAME_LENGTH 255

class Settings : public Config::RegistryGroup
{
public:
   Settings(void);

   static DECLARE_SETTING(WindowPosition, RECT);
   static DECLARE_SETTING(DockedBorders, int);
   static DECLARE_SETTING(ColumnNumber, unsigned long);
   static DECLARE_SETTING(LockPreviewWindow, bool);
   static DECLARE_SETTING(ShowWindow, bool);
   static DECLARE_SETTING(HasTrayIcon, bool);
   static DECLARE_SETTING(AlwaysOnTop, bool);
   static DECLARE_SETTING(TransparencyLevel, unsigned char);
   static DECLARE_SETTING(HasCaption, bool);
   static DECLARE_SETTING(SnapSize, int);
   static DECLARE_SETTING(AutoHideDelay, int);
   static DECLARE_SETTING(EnableToolTips, bool);
   static DECLARE_SETTING(ConfirmKilling, bool);
   static DECLARE_SETTING(AutoSaveWindowSettings, bool);
   static DECLARE_SETTING(CloseToTray, bool);
   static DECLARE_SETTING(AutoSwitchDesktop, bool);
   static DECLARE_SETTING(AllWindowsInTaskList, bool);
   static DECLARE_SETTING(IntegrateWithShell, bool);
   static DECLARE_SETTING(SwitchToNextDesktopHotkey, int);
   static DECLARE_SETTING(SwitchToPreviousDesktopHotkey, int);
   static DECLARE_SETTING(SwitchToTopDesktopHotkey, int);
   static DECLARE_SETTING(SwitchToBottomDesktopHotkey, int);
   static DECLARE_SETTING(SwitchToLeftDesktopHotkey, int);
   static DECLARE_SETTING(SwitchToRightDesktopHotkey, int);
   static DECLARE_SETTING(MoveWindowToNextDesktopHotkey, int);
   static DECLARE_SETTING(MoveWindowToPreviousDesktopHotkey, int);
   static DECLARE_SETTING(MoveWindowToDesktopHotkey, int);
   static DECLARE_SETTING(MaximizeHeightHotkey, int);
   static DECLARE_SETTING(MaximizeWidthHotkey, int);
   static DECLARE_SETTING(AlwaysOnTopHotkey, int);
   static DECLARE_SETTING(TransparencyHotkey, int);
   static DECLARE_SETTING(TogglePreviewWindowHotkey, int);
   static DECLARE_SETTING(DisplayMode, int);
   static DECLARE_SETTING(BackgroundColor, COLORREF);
   static DECLARE_SETTING(BackgroundPicture, LPTSTR);
   static DECLARE_SETTING(DesktopNameOSD, bool);
   static DECLARE_SETTING(PreviewWindowFont, LOGFONT);
   static DECLARE_SETTING(PreviewWindowFontColor, COLORREF);
   static DECLARE_SETTING(OSDTimeout, int);
   static DECLARE_SETTING(OSDFont, LOGFONT);
   static DECLARE_SETTING(OSDFgColor, COLORREF);
   static DECLARE_SETTING(OSDBgColor, COLORREF);
   static DECLARE_SETTING(OSDPosition, POINT);
   static DECLARE_SETTING(OSDTransparencyLevel, unsigned char);
   static DECLARE_SETTING(OSDHasBackground, bool);
   static DECLARE_SETTING(OSDIsTransparent, bool);
   static DECLARE_SETTING(WarpEnable, bool);
   static DECLARE_SETTING(WarpSensibility, LONG);
   static DECLARE_SETTING(WarpMinDuration, DWORD);
   static DECLARE_SETTING(WarpRewarpDelay, DWORD);
   static DECLARE_SETTING(WarpRequiredVKey, int);
   static DECLARE_SETTING(WarpInvertMousePos, bool);
   static DECLARE_SETTING(DefaultHidingMethod, int);
   static DECLARE_SETTING(LanguageCode,int);

   // Other settings
   Config::Group * GetShellIntegrationExceptions() { return GetSubGroup(regSubKeyDisableShellIntegration); }
   Config::Group * GetHidingMethodExceptions()     { return GetSubGroup(regSubKeyHidingMethods); }

   bool LoadStartWithWindows();
   void SaveStartWithWindows(bool start);
   bool LoadDisableShellIntegration(const char * windowclass);
   void SaveDisableShellIntegration(const char * windowclass, bool enable);
   int LoadHidingMethod(const char * windowclass);
   void SaveHidingMethod(const char * windowclass, int method);

   class SubkeyList: public Config::RegistryGroup
   {
   public:
      SubkeyList(Settings * settings, const char regKey[]);
      SubkeyList(Settings * settings, const char regKey[], int index);
      SubkeyList(Settings * settings, const char regKey[], char * name, bool create=true);

      virtual bool Open(int index);
      virtual bool Open(const char * name, bool create=true);

      bool IsValid();
      void Destroy();

      char * GetName(char * buffer, unsigned int length);
      bool Rename(char * buffer);

   protected:
      char m_name[MAX_NAME_LENGTH];

      Config::RegistryGroup m_group;
   };

   class Desktop: public SubkeyList
   {
   public:
      Desktop(Settings * settings): SubkeyList(settings, regKeyDesktops)                     {}
      Desktop(Settings * settings, int index): SubkeyList(settings, regKeyDesktops, index)   {}
      Desktop(Settings * settings, char * name): SubkeyList(settings, regKeyDesktops, name)  {}

      static DECLARE_SETTING(DeskIndex, int);
      static DECLARE_SETTING(DeskWallpaper, LPTSTR);
      static DECLARE_SETTING(DeskHotkey, int);
      static DECLARE_SETTING(BackgroundColor, COLORREF);

   protected:
      static const char regKeyDesktops[];
   };

   class Window: public SubkeyList
   {
   public:
      Window(Settings * settings): SubkeyList(settings, regKeyWindows)                       {}
      Window(Settings * settings, int index): SubkeyList(settings, regKeyWindows, index) {}
      Window(Settings * settings, char * name, bool create=false): SubkeyList(settings, regKeyWindows, name, create)  {}

      bool OpenDefault()                                       { return Open(NULL); }
      virtual bool Open(const char * name, bool create=false)  { return SubkeyList::Open(name, create); }

      static DECLARE_SETTING(AlwaysOnTop, bool);
      static DECLARE_SETTING(OnAllDesktops, bool);
      static DECLARE_SETTING(MinimizeToTray, bool);
      static DECLARE_SETTING(TransparencyLevel, unsigned char);
      static DECLARE_SETTING(EnableTransparency, bool);
      static DECLARE_SETTING(AutoSaveSettings, bool);
      static DECLARE_SETTING(WindowPosition, RECT);
      static DECLARE_SETTING(AutoSetSize, bool);
      static DECLARE_SETTING(AutoSetPos, bool);
      static DECLARE_SETTING(AutoSetDesk, bool);
      static DECLARE_SETTING(DesktopIndex, int);

   protected:
      static const char regKeyWindows[];
   };

protected:
   static const char regKeyName[];
   static const char regKeyWindowsStartup[];

   static const char regSubKeyDisableShellIntegration[];
   static const char regSubKeyHidingMethods[];
   static const char regValStartWithWindows[];

   static DWORD LoadDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD defVal);
   static void SaveDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD value);
   static bool LoadBinary(HKEY regKey, bool keyOpened, const char * entry, LPBYTE buffer, DWORD length);
   static void SaveBinary(HKEY regKey, bool keyOpened, const char * entry, LPBYTE buffer, DWORD length);

   friend class Settings::Desktop;
   friend class Settings::Window;
};

#endif /*__SETTINGS_H__*/
