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

#ifndef __CMDLINE_H__
#define __CMDLINE_H__

#include <map>
#include "Locale.h"


/** Option for the command line parser.
 * This clas encapsulates an option for the command line parser. For each option,
 * a new class inheriting from this one should be created. It will be automatically
 * registered with the command line parser once it is instanciated, and the
 * ParseOption() method will be called if the option is encountered.
 */
class CommandLineOption {
public:
   enum ArgType { no_argument, required_argument, optional_argument };

   /** Create the option.
    * @param opcode The opcode used to trigger the option.
    * @param resid The resource id associated with this option. This is used for
    * showing command line usage.
    * @param arg Type of argument: none, required, or optional.
    */
   CommandLineOption(char opcode, UINT resid, ArgType arg = no_argument);

   virtual ~CommandLineOption()		{}

   /** Parse the option.
    * This method is called whenever the option is encountered during the parsing
    * of the command line.
    * @param arg The argument of the option, or NULL if no argument is provided.
    */
   virtual void ParseOption(LPCTSTR arg = NULL) = 0;

   UINT GetResId() const         { return m_resid; }
   ArgType GetArgType() const    { return m_argType; }


   /** Show a dialog box with the usage of the options.
    */
   static void ShowUsage();

   /** Register an option.
    * This setp is needed for the option to be recognized.
    */
   static void RegisterOption(char opcode, CommandLineOption* option);

   /** Get an option.
	*/
   static CommandLineOption * GetOption(char opcode);

protected:
   UINT m_resid;
   ArgType m_argType;

   static std::map<char, CommandLineOption*> s_argsmap;	/// List of all options
};

/** Simple option class to parse a flag.
 * When the flag is detected, the given variable is set to the specified value.
 */
class CommandLineFlag : public CommandLineOption {
public:
   CommandLineFlag(char opcode, UINT resid):
      CommandLineOption(opcode, resid), m_flag(false)
   {}

   virtual void ParseOption(LPCTSTR /*arg*/)	{ m_flag = true; }
   operator bool()							    { return m_flag; }

protected:
   bool m_flag;
};

/** Simple option class to parse an integer.
 * When the flag is detected, the given variable is set to the parsed value.
 */
class CommandLineInt : public CommandLineOption {
public:
   CommandLineInt(char opcode, UINT resid, int defval=0, int optval=1, ArgType type=required_argument):
      CommandLineOption(opcode, resid, type), m_flag(defval), m_optval(optval)
   {}

   virtual void ParseOption(LPCTSTR arg)
   {
		if (arg)
		{
			char * ptr;
			int val = strtol(arg, &ptr, 0);
			if (ptr != arg)
				m_flag = val;
		}
		else
			m_flag = m_optval;
   }

   operator int()							{ return m_flag; }

protected:
   int m_flag;
   int m_optval;
};

/** Simple option class to parse a string.
 * When the flag is detected, the given variable is set to the parsed value.
 */
class CommandLineStr : public CommandLineOption {
public:
   CommandLineStr(char opcode, UINT resid, LPCTSTR defval="", LPCTSTR optval="", ArgType type=required_argument):
      CommandLineOption(opcode, resid, type), m_str(defval), m_optval(optval)
   {}

   virtual void ParseOption(LPCTSTR arg)	{ m_str = arg ? arg : m_optval; }
   operator String()						{ return m_str; }

protected:
   String m_str;
   LPCTSTR m_optval;
};

/** Command line parser class.
 * This class can be used to parse command line arguments. It parses the given
 * string for the options that have been created so far (see CommandLineOption
 * class).
 */
class CommandLineParser
{
public:
   CommandLineParser();
   ~CommandLineParser();

   /** Parse command line.
    * This is the main entry point. It parses the given command line, and handles
    * each option.
    */
   bool ParseCommandLine(LPTSTR cmdline);

protected:
   bool ProcessArg(LPCTSTR arg);
   bool EndParsing();

   enum { NONE, REQARG, OPTARG} m_argState;
   CommandLineOption * m_curOption;
};

#endif /*__CMDLINE_H__*/
