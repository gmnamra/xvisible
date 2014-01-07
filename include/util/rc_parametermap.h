/*------------------------------------------------------
   rcCmdLine

   A utility for parsing command lines.

   Copyright (C) 1999 Chris Losinger, Smaller Animals Software.
   http://www.smalleranimals.com

   This software is provided 'as-is', without any express
   or implied warranty.  In no event will the authors be 
   held liable for any damages arising from the use of this software.

   Permission is granted to anyone to use this software 
   for any purpose, including commercial applications, and 
   to alter it and redistribute it freely, subject to the 
   following restrictions:

     1. The origin of this software must not be misrepresented; 
   you must not claim that you wrote the original software. 
   If you use this software in a product, an acknowledgment 
   in the product documentation would be appreciated but is not required.
   
     2. Altered source versions must be plainly marked as such, 
   and must not be misrepresented as being the original software.
   
     3. This notice may not be removed or altered from any source 
   distribution.

  -------------------------

   Example :

   Our example application uses a command line that has two
   required switches and two optional switches. The app should abort
   if the required switches are not present and continue with default
   values if the optional switches are not present.

      Sample command line : 
      MyApp.exe -p1 text1 text2 -p2 "this is a big argument" -opt1 -55 -opt2

      Switches -p1 and -p2 are required. 
      p1 has two arguments and p2 has one.
      
      Switches -opt1 and -opt2 are optional. 
      opt1 requires a numeric argument. 
      opt2 has no arguments.
      
      Also, assume that the app displays a 'help' screen if the '-h' switch
      is present on the command line.

   #include "CmdLine.h"

   void main(int argc, char **argv)
   {
      // our cmd line parser object
      rcCmdLine cmdLine;

      // parse argc,argv 
      if (cmdLine.SplitLine(argc, argv) < 1)
      {
         // no switches were given on the command line, abort
         ASSERT(0);
         exit(-1);
      }

      // test for the 'help' case
      if (cmdLine.hasSwitch("-h"))
      {
         show_help();
         exit(0);
      }

      // get the required arguments
      string p1_1, p1_2, p2_1;
      try
      {  
         // if any of these fail, we'll end up in the catch() block
         p1_1 = cmdLine.GetArgument("-p1", 0);
         p1_2 = cmdLine.GetArgument("-p1", 1);
         p2_1 = cmdLine.GetArgument("-p2", 0);

      }
      catch (...)
      {
         // one of the required arguments was missing, abort
         ASSERT(0);
         exit(-1);
      }

      // get the optional parameters

      // convert to an int, default to '100'
      int iOpt1Val =    atoi(cmdLine.GetSafeArgument("-opt1", 0, 100));

      // since opt2 has no arguments, just test for the presence of
      // the '-opt2' switch
      bool bOptVal2 =   cmdLine.hasSwitch("-opt2");

      .... and so on....

   }


------------------------------------------------------*/
#ifndef rcPARAM_DEF
#define rcPARAM_DEF

#include <rc_types.h>

#include <iostream> // you may need this
#include <map>
#include <string>
#include <vector>
using namespace std ;

// handy little container for our argument vector
struct rcCmdParam
{
   vector<string> m_strings;
};

// this class is actually a map of strings to vectors
typedef map<string, rcCmdParam> rcCmdLineMap;

// the command line parser class
class rcCmdLine : public rcCmdLineMap
{

public:
   /*------------------------------------------------------
      int rcCmdLine::SplitLine(int argc, char **argv)

      parse the command line into switches and arguments.

      returns number of switches found
   ------------------------------------------------------*/
   int         splitLine(int argc, char **argv);

   /*------------------------------------------------------
      bool rcCmdLine::hasSwitch(const char *pSwitch)

      was the switch found on the command line ?

      ex. if the command line is : app.exe -a p1 p2 p3 -b p4 -c -d p5

      call                          return
      ----                          ------
      cmdLine.hasSwitch("-a")       true
      cmdLine.hasSwitch("-z")       false
   ------------------------------------------------------*/   
   bool        hasSwitch(const char *pSwitch);

   /*------------------------------------------------------

      string rcCmdLine::getSafeArgument(const char *pSwitch, int iIdx, const char *pDefault)

      fetch an argument associated with a switch . if the parameter at
      index iIdx is not found, this will return the default that you
      provide.

      example :
  
      command line is : app.exe -a p1 p2 p3 -b p4 -c -d p5

      call                                      return
      ----                                      ------
      cmdLine.getSafeArgument("-a", 0, "zz")    p1
      cmdLine.getSafeArgument("-a", 1, "zz")    p2
      cmdLine.getSafeArgument("-b", 0, "zz")    p4
      cmdLine.getSafeArgument("-b", 1, "zz")    zz

   ------------------------------------------------------*/

   string  getSafeArgument(const char *pSwitch, int iIdx, const char *pDefault);

   /*------------------------------------------------------

      string rcCmdLine::getArgument(const char *pSwitch, int iIdx)

      fetch a argument associated with a switch. throws an exception 
      of (int)0, if the parameter at index iIdx is not found.

      example :
  
      command line is : app.exe -a p1 p2 p3 -b p4 -c -d p5

      call                             return
      ----                             ------
      cmdLine.getArgument("-a", 0)     p1
      cmdLine.getArgument("-b", 1)     throws (int)0, returns an empty string

   ------------------------------------------------------*/
   string  getArgument(const char *pSwitch, int iIdx); 

   /*------------------------------------------------------
      int rcCmdLine::getArgumentCount(const char *pSwitch)

      returns the number of arguments found for a given switch.

      returns -1 if the switch was not found

   ------------------------------------------------------*/
   int         getArgumentCount(const char *pSwitch);

   int32 atoi (string s)
   {
     return std::atoi (s.c_str());
   }

protected:
   /*------------------------------------------------------

   protected member function
   test a parameter to see if it's a switch :

   switches are of the form : -x
   where 'x' is one or more characters.
   the first character of a switch must be non-numeric!

   ------------------------------------------------------*/
   bool        isSwitch(const char *pParam);
};

#endif rcPARAM_DEF
