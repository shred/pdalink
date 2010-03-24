/**
 * PdaLink -- Connect Palm with Amiga
 *
 * Library initialization, C part
 *
 * (C) 1998-2000 Richard Körber <rkoerber@gmx.de>
 *
 *------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * You must not use this source code to gain profit of any kind!
 */

#include "pdalink_glob.h"

/*----------------------------------------------------------------
** GLOBAL VARIABLES
*/
struct DOSBase       *DOSBase;
struct Library       *UtilityBase;
struct Library       *SysBase;
struct Library       *RexxSysBase;
struct Library       *PdalinkBase;

struct Global         global;


/*----------------------------------------------------------------
** InitGlobal   Initializes the global structure
*/
static int InitGlobal(void)
{
  for(;;)
  {
    if(!(global.slptrash = AllocMem(1024,MEMF_PUBLIC))) break;

    NewList(&global.rexxHandles);
    InitSemaphore(&global.rexxHandleSemaphore);
    global.rexxHandleOK = TRUE;

    return 1;                   // Okay
  }
  return 0;                     // Failure
}

/*----------------------------------------------------------------
** ExitGlobal   Frees global structure
*/
static void ExitGlobal(void)
{
  struct RexxHandle *thisNode, *nextNode;

  if(global.slptrash) FreeMem(global.slptrash,1024);

  if(global.rexxHandleOK)
  {
    global.rexxHandleOK = FALSE;
    ObtainSemaphore(&global.rexxHandleSemaphore);
    for                                                     // Clean up the handle list
    (
      thisNode = (struct RexxHandle *)global.rexxHandles.lh_Head;
      nextNode = (struct RexxHandle *)thisNode->rh_node.ln_Succ;
      thisNode = nextNode
    )
    {
      Remove((struct Node *)thisNode),
      FreeVec(thisNode);
    }
  }

  return;
}


/*----------------------------------------------------------------
** LibExit      Finishes Library
*/
__saveds __asm void LibExit(void)
{
  ExitGlobal();
  if(DOSBase)       CloseLibrary((struct Library *)DOSBase);
  if(UtilityBase)   CloseLibrary(                  UtilityBase);
  if(RexxSysBase)   CloseLibrary(                  RexxSysBase);
}


/*----------------------------------------------------------------
** LibInit      Initializes the Library
*/
__saveds __asm struct Library *LibInit
(
  register __a0 struct Library *exec,
  register __a1 struct Library *base
)
{
  SysBase      = exec;
  PdalinkBase = base;

  for(;;)
  {
    if(!(DOSBase       = (struct DOSBase *)      OpenLibrary("dos.library"        ,36L))) break;
    if(!(UtilityBase   =                         OpenLibrary("utility.library"    ,36L))) break;
    if(!(RexxSysBase   =                         OpenLibrary("rexxsyslib.library" ,36L))) break;

    if(!(InitGlobal()))   break;

    return(base);         // Everything worked fine
  }
  LibExit();
  return(0);
}

/****************************************************************/

