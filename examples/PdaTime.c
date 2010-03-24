/*********************************************************************
**                                                                  **
**        PdaTime         -- Set Amiga or Pilot system time         **
**                                                                  **
*********************************************************************/
/*
**  Copyright � 1998-2000 Richard K�rber  --  All Rights Reserved
**    E-Mail: rkoerber@gmx.de
**    URL:    http://shredzone.home.pages.de
**
***************************************************************/
/*
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**  The author (Richard K�rber) reserves the right to revoke the
**  GNU General Public License whenever he feels that it is necessary,
**  especially when he found out that the licence has been abused,
**  ignored or violated, and without prior notice.
**
**  You must not use this source code to gain profit of any kind!
**
***************************************************************/
/*
**  Compiles with SAS/C, e.g.
**      sc PdaTime.c NOSTACKCHECK DATA=NEAR STRMER CPU=68060 OPT
*/

#include <stdio.h>
#include <string.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/pdalink_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/pdalink_pragmas.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosasl.h>
#include <libraries/pdalink.h>


#define  VERSIONSTR   "0.3�"
#define  DATESTR      "7.1.2000"
#define  COPYRIGHTSTR "2000"
#define  EMAILSTR     "rkoerber@gmx.de"
#define  URLSTR       "http://shredzone.home.pages.de"

#define  NORMAL       "\2330m"
#define  BOLD         "\2331m"
#define  ITALIC       "\2333m"
#define  UNDERLINE    "\2334m"

#define  MKTAG(a,b,c,d)  ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))

static char ver[] = "$VER: PdaTime " VERSIONSTR " (" DATESTR ") " EMAILSTR;
static char titletxt[] = \
  BOLD "PdaTime " VERSIONSTR " (C) " COPYRIGHTSTR " Richard K�rber -- all rights reserved" NORMAL "\n"
  BOLD "  WARNING:" NORMAL " This is an early beta release. If you don't\n"
  "    know what this means, do " BOLD "NOT" NORMAL " use this program!\n\n";
static char helptxt[] = \
  "  " ITALIC "E-Mail: " NORMAL EMAILSTR "\n"
  "  " ITALIC "URL:    " NORMAL URLSTR "\n\n"
  ITALIC "Usage:" NORMAL "\n"
  "  DEVICE/K       Serial device (\"serial.device\")\n"
  "  UNIT/K/N       Serial unit (0)\n"
  "  MAXBAUD/K/N    Maximum baud (57600)\n"
  "\n";

struct Parameter
{
  STRPTR device;
  LONG   *unit;
  LONG   *maxbaud;
}
param;

static char template[] = "SD=DEVICE/K,SU=UNIT/K/N,SB=MAXBAUD/K/N";

extern struct DOSBase *DOSBase;
struct Library *PdalinkBase;
struct Library *UtilityBase;
APTR socket = NULL;
BOOL ignoreError = FALSE;



/*
** Open a connection to the Pilot
*/
int Connect(void)
{
  LONG error;

  if(socket) return(TRUE);

  socket = PL_OpenSocketTags
    (
    PLTAG_ErrorPtr      , &error,
    (param.device  ? PLTAG_SerialDevice  : TAG_IGNORE) , param.device,
    (param.unit    ? PLTAG_SerialUnit    : TAG_IGNORE) , *param.unit ,
    (param.maxbaud ? PLTAG_SerialMaxRate : TAG_IGNORE) , *param.maxbaud,
    TAG_DONE
    );

  if(socket)
  {
    Printf("Please press the HotSync button " ITALIC "now" NORMAL "\n");
    if(PL_Accept(socket,10L)) {
      Printf("Connection established, %ld bps\n",PL_GetBaudRate(socket));
      return(TRUE);
    }
  }
  else
  {
    Printf("** Socket error %ld\n",error);
  }
  return(FALSE);
}

/*
** Close all databases and disconnect
*/
void Disconnect(void)
{
  if(!socket) return;
  DLP_AddSyncLogEntry(socket,"-- AMIGA made it possible --\n");
  DLP_EndOfSync(socket,0);
  PL_CloseSocket(socket);
}

/*
** Show Pilot time
*/
void cmd_show(void)
{
  struct DLP_SysTime time;

  if(!Connect()) return;
  if(!DLP_OpenConduit(socket)) return;
  if(!DLP_GetSysTime(socket,&time))
  {
    PutStr("** Couldn't get time\n");
    return;
  }

  Printf("%02ld.%02ld.%04ld %02ld:%02ld:%02ld\n",time.day,time.month,time.year,time.hour,time.minute,time.second);
}

/*
** MAIN PART
*/
int main(void)
{
  struct RDArgs *args;

  PutStr(titletxt);

  if(args = (struct RDArgs *)ReadArgs(template,(LONG *)&param,NULL))
  {
    if(UtilityBase = OpenLibrary("utility.library",36L))
    {
      if(PdalinkBase = OpenLibrary("pdalink.library",0L))
      {
        cmd_show();

        if(socket)
        {
          if(!ignoreError && (PL_LastError(socket)!=0))
            Printf("** Socket error code %ld\n",PL_LastError(socket));
          Disconnect();
        }
        CloseLibrary(PdalinkBase);
      }
      else PutStr("** Couldn't open pdalink.library\n");
      CloseLibrary(UtilityBase);
    }
    else PutStr("** Couldn't open utility.library\n");

    FreeArgs(args);
  }
  else PutStr(helptxt);

  return(0);
}

/********************************************************************/
