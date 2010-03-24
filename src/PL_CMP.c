/**
 * PdaLink -- Connect Palm with Amiga
 *
 * CMP (Connection Management Protocol)
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

/*------------------------------------------------------**
** Name:        PL_CMPAbort                       public
**
** Funktion:    CMP: Verbindung abbrechen
**
** Parameter:   socket    Socket für die Übertragung
**              reason    Grund-Code
** Ergebnis:    success   Erfolg (boolean)
//>
** Bemerkungen:
**
** Revision:     7. Juni 1998, 22:49:33
*/
__saveds __asm int PL_CMPAbort
(
  register __a0 APTR socket,
  register __d0 UBYTE reason
)
{
  struct PL_Socket *sock = (struct PL_Socket *)socket;
  struct PL_CMP cmp = { PLCMP_WAKEUP,0,0,0,0 };
  KPRINTF(10, ("CMPAbort\n"));

  sock->lastError = PLERR_OKAY;
  cmp.flags = reason;
  if(sock->serial->isUSB)
  {
      return(TRUE);
  }
  return(-1!=PL_PADPWrite(socket,&cmp,sizeof(struct PL_CMP),PLPADP_DATA));
}
//<

/*------------------------------------------------------**
** Name:        PL_CMPInit                        public
**
** Funktion:    CMP: Verbindung herstellen
**
** Parameter:   socket    Socket für die Übertragung
**              baud      Baudrate
** Ergebnis:    success   Erfolg (boolean)
//>
** Bemerkungen: Die serielle Schnittstelle wird nicht
**              auf die neue Baudrate gesetzt.
**
** Revision:     7. Juni 1998, 22:49:33
*/
__saveds __asm int PL_CMPInit
(
  register __a0 APTR socket,
  register __d0 ULONG rate
)
{
  struct PL_Socket *sock = (struct PL_Socket *)socket;
  struct PL_CMP cmp = { PLCMP_INIT,0,0,0,0 };
  KPRINTF(10, ("CMPInit\n"));

  sock->lastError = PLERR_OKAY;
  cmp.baudrate = rate;
  if(rate!=9600) cmp.flags = PLCMPF_CHANGEBAUD;
  if(sock->serial->isUSB)
  {
      return(TRUE);
  }
  return(-1!=PL_PADPWrite(socket,&cmp,sizeof(struct PL_CMP),PLPADP_DATA));
}
//<

/*------------------------------------------------------**
** Name:        PL_CMPWakeUp                      public
**
** Funktion:    CMP: Aufwecken
**
** Parameter:   socket    Socket für die Übertragung
**              maxbaud   Maximale Baudrate
** Ergebnis:    success   Erfolg (boolean)
//>
** Bemerkungen: Die serielle Schnittstelle wird nicht
**              auf die neue Baudrate gesetzt.
**
** Revision:     7. Juni 1998, 23:00:59
*/
__saveds __asm int PL_CMPWakeUp
(
  register __a0 APTR socket,
  register __d0 ULONG maxrate
)
{
  struct PL_Socket *sock = (struct PL_Socket *)socket;
  struct PL_CMP cmp = { PLCMP_WAKEUP,0,PLCMPVER_1_0,0,0 };
  KPRINTF(10, ("CMPWakeUp\n"));

  sock->lastError = PLERR_OKAY;
  cmp.baudrate = maxrate;
  if(sock->serial->isUSB)
  {
      return(TRUE);
  }
  return(-1!=PL_PADPWrite(socket,&cmp,sizeof(struct PL_CMP),PLPADP_WAKE));
}
//<

/*------------------------------------------------------**
** Name:        PL_CMPRead                        public
**
** Funktion:    CMP: lesen
**
** Parameter:   socket    Socket für die Übertragung
**              cmp       CMP-Struktur-Puffer
** Ergebnis:    success   Erfolg (boolean)
//>
** Bemerkungen:
**
** Revision:     7. Juni 1998, 23:04:21
*/
__saveds __asm int PL_CMPRead
(
  register __a0 APTR socket,
  register __d0 struct PL_CMP *cmp
)
{
  struct PL_Socket *sock = (struct PL_Socket *)socket;
  KPRINTF(10, ("CMPRead\n"));

  sock->lastError = PLERR_OKAY;
  if(sock->serial->isUSB)
  {
      return(TRUE);
  }
  return(-1!=PL_PADPRead(socket,cmp,sizeof(struct PL_CMP)));
}
//<


/********************************************************************/
