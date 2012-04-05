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
#include "CmdLine.h"
#include "StdString.h"

using namespace std;

map<char, CommandLineOption*> CommandLineOption::s_argsmap;

CommandLineOption::CommandLineOption(char opcode, UINT resid, ArgType arg): m_resid(resid), m_argType(arg)
{
   CommandLineOption::RegisterOption(opcode, this);
}

CommandLineParser::CommandLineParser()
{
}

CommandLineParser::~CommandLineParser()
{
}

bool CommandLineParser::ProcessArg(LPCTSTR arg)
{
   CommandLineOption * option;
   bool res = true;

   switch(m_argState)
   {
   case NONE:
      if (arg[0] != '-')
         res = (MessageBox(NULL, arg, "No an option", MB_ICONERROR), false); //no an option
      else if ((m_curOption = CommandLineOption::GetOption(arg[1])) == NULL)
         res = (MessageBox(NULL, arg, "Invalid option", MB_ICONERROR), false); //invalid option
      else if (m_curOption->GetArgType() == CommandLineOption::required_argument)
         m_argState = REQARG;
      else if (m_curOption->GetArgType() == CommandLineOption::optional_argument)
         m_argState = OPTARG;
      else
         m_curOption->ParseOption();
      break;

   case REQARG:
      m_curOption->ParseOption(arg);
      m_argState = NONE;
      break;

   case OPTARG:
      if (arg[0] == '-' && (option = CommandLineOption::GetOption(arg[1])) != NULL)
      {
         m_curOption->ParseOption();   //no argument !

         m_curOption = option;
         if (m_curOption->GetArgType() == CommandLineOption::required_argument)
            m_argState = REQARG;
         else if (m_curOption->GetArgType() == CommandLineOption::optional_argument)
            m_argState = OPTARG;
         else
         {
            m_curOption->ParseOption();
            m_argState = NONE;
         }
      }
      else
      {
         m_curOption->ParseOption(arg);
         m_argState = NONE;
      }
      break;
   }

   return res;
}

bool CommandLineParser::EndParsing()
{
   bool res = true;

   switch(m_argState)
   {
   case NONE:
      //nothing to be done
      break;

   case REQARG:
      MessageBox(NULL, "The last argument needs an argument, which is not provided", "Invalid command line", MB_ICONERROR);
      res = false;
      break;

   case OPTARG:
      m_curOption->ParseOption();
      break;
   }

   return res;
}

bool CommandLineParser::ParseCommandLine(LPTSTR cmdline)
{
#define ESCAPE_CHAR  '\\'
#define QUOTE_CHAR   '"'

   enum { IDLE, ARG, STRARG } state = IDLE;
   enum { SPACE, QUOTE, OTHER } token;
   TCHAR value;
   CStdString arg;
   bool res = true;

   m_argState = NONE;

   while(res && *cmdline)
   {
      value = *cmdline++;

      //Tokenizer
      if (value == ESCAPE_CHAR)
      {
         //Beginning of an escape char. The actual char is the next one.
         //If this is the end of the string, handle the escape as a regular character.
         if (*cmdline)
            value = *cmdline++;
         token = OTHER;
      }
      else if (isspace(value))
         token = SPACE;
      else if (value == QUOTE_CHAR)
         token = QUOTE;
      else
         token = OTHER;

      //Parser
      switch(state)
      {
      case IDLE:
         if (token == QUOTE)
         {
            arg = "";
            state = STRARG;
         }
         else if (token != SPACE)
         {
            arg = value;
            state = ARG;
         }
         break;

      case ARG:
         if (token == SPACE)
         {
            res = ProcessArg(arg);
            arg = "";
         }
         else if (token == QUOTE)
            state = STRARG;
         else
            arg += value;
         break;

      case STRARG:
         if (token == QUOTE)
            state = ARG;
         else
            arg += value;
         break;
      }
   }

   //Process the last argument, which may not have been processed yet
   //(e.g. if it was not followed by spaces)
   if (res && arg != "")
      res = ProcessArg(arg);

   if (res)
      res = EndParsing();

#undef ESCAPE_CHAR
#undef QUOTE_CHAR

	return res;
}

CommandLineOption * CommandLineOption::GetOption(char opcode)
{
   map<char, CommandLineOption*>::iterator it = s_argsmap.find(opcode);
   return it == s_argsmap.end() ? NULL : (*it).second;
}

void CommandLineOption::RegisterOption(char opcode, CommandLineOption* option)
{
   s_argsmap[opcode] = option;
}

