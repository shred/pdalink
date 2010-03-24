/**
 * PdaLink -- Connect Palm with Amiga
 *
 * PADP (Packet Assembly Disassembly Protocol)
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
** Name:        PL_PADPWrite                      public
**
** Funktion:    Verschickt ein PADP-Paket
**
** Parameter:   socket    Socket für die Übertragung
**              buffer    zu übertragener Puffer
**              length    Länge des Puffers
**              type      Zu übertragener Typ
** Ergebnis:    length    Geschriebene Bytes, oder
**                        -1: Fehler
//>
** Bemerkungen:
**
** Revision:    31. Mai 1998, 21:14:30
*/
__saveds __asm LONG PL_PADPWrite
(
  register __a0 APTR socket,
  register __a1 APTR buffer,
  register __d0 LONG length,
  register __d1 UWORD type
)
{
  struct PL_Socket *sock = (struct PL_Socket *)socket;
  struct PL_SLP_Header slp;       // SLP header
  struct PL_SLP_Header recslp;    // SLP header to be received
  struct PL_PADP_Header padp;     // PADP header
  UBYTE flags = PLPADPF_FIRST;
  LONG  remain = length;          // bytes remaining to be sent
  LONG  offset = 0;
  LONG  plen;

  sock->lastError = PLERR_OKAY;

  /* Create transaction ID */
  if(type==PLPADP_WAKE)
  {
    sock->transactionID = 0xFF;
  }
  else if(type!=PLPADP_ACK && !sock->initiator)
  {
    sock->transactionID++;
    if((sock->transactionID==0x00) || (sock->transactionID==0xFF)) // wrap around in time
      sock->transactionID=0x01;
  }
  else if(sock->transactionID==0x00)
  {
    sock->transactionID = 0x10;   // or another value
  }

  /* create SLP header */
  slp.destSocket = PLSLPSOCK_DLP;
  slp.srcSocket  = PLSLPSOCK_DLP;
  slp.pckType    = PLSLPTYPE_PADP;
  slp.transID    = sock->transactionID;

  /* transfer data */
  do
  {
    plen = min(remain,1024);

    /* create PADP header */
    padp.type      = type & 0xFF;
    padp.flags     = (remain==plen ? flags|PLPADPF_LAST : flags);
    padp.size      = (flags!=0 ? length : offset);

#ifdef PADPDEBUG
  Printf("PADP A->P: type=%ld flags=0x%02lx size=%ld tID=0x%02lx\n",padp.type,padp.flags,padp.size,slp.transID);
#endif


    /* send data */
    if(PL_SLPWrite(socket,((UBYTE *)buffer)+offset,plen,&slp,&padp) == -1)
    {
      offset = -1;               // error
      break;
    }

    /* eventually wait for ACK */
    if(type != PLPADP_TICKLE)   // tickles are not acknowledged
    {
      for(;;)
      {
        /*TODO: this is ugly, because it is not error tolerant */

        if(!PL_SLPRead(socket,global.slptrash,1024,&recslp,&padp) == -1)
        {
          offset = -1;             // error
          goto error;
        }

        if(padp.flags & PLPADPF_MEMERROR)
        {
          if(slp.transID == recslp.transID)
          {
            sock->lastError = PLERR_REMOTENOMEM;
            offset = -1;
            goto error;
          }
          else continue;          // bad packet at wrong time
        }

        if(padp.type == PLPADP_TICKLE)
          continue;               // tickles are ignored

        if(   (padp.type != PLPADP_ACK)
           || (slp.transID != recslp.transID))
        {
          sock->lastError = PLERR_BADPACKET;
          offset = -1;
#ifdef DEBUGOUT
  Printf("DEBUG: File %s Line %ld: Bad Packet\n",__FILE__,__LINE__);
  Printf("  padp.type = %ld\n  slp.transID = %ld\n  recslp.transID = %ld\n",padp.type,slp.transID,recslp.transID);
  Printf("  slptrash = %08lx %08lx %08lx %08lx\n",((ULONG *)global.slptrash)[0],((ULONG *)global.slptrash)[1],((ULONG *)global.slptrash)[2],((ULONG *)global.slptrash)[3]);
#endif
          goto error;             // Our packet is not acknowledged?!
        }

#ifdef PADPDEBUG
  Printf("PADP A->P: [received good ACK]\n");
#endif

        break;                    // Everything is fine: next packet
      }
    }

    offset += plen;
    remain -= plen;
    flags  = 0;                   // Not the FIRST any more
  }
  while(remain>0);

error:
  if(type!=PLPADP_ACK && sock->initiator)
  {
    sock->transactionID++;
    if(sock->transactionID==0xFF)
      sock->transactionID=0x01;
  }

  return(offset);
}
//<

/*------------------------------------------------------**
** Name:        PL_PADPRead                       public
**
** Funktion:    Holt ein PADP-Paket
**
** Parameter:   socket    Socket für die Übertragung
**              buffer    Empfangspuffer
**              length    Pufferlänge
** Ergebnis:    length    Länge des empfangenen Puffers,
**                        -1: Fehler
//>
** Bemerkungen:
**
** Revision:     1. Juni 1998, 17:33:41
*/
__saveds __asm LONG PL_PADPRead
(
  register __a0 APTR socket,
  register __a1 APTR buffer,
  register __d0 LONG length
)
{
  struct PL_Socket *sock = (struct PL_Socket *)socket;

  struct PL_PADP_Header padp;
  struct PL_SLP_Header  slp;
  struct PL_PADP_Header ackpadp;
  struct PL_SLP_Header  ackslp =
  {
    0,0,0,
    PLSLPSOCK_DLP,
    PLSLPSOCK_DLP,
    PLSLPTYPE_PADP,
    0,
    0,
    0
  };

  LONG   offset = 0;
  LONG   gotoffset;
  LONG   remain = length;
  LONG   read;

  sock->lastError = PLERR_OKAY;

  /* Create transaction ID */
  if(sock->initiator)
  {
    sock->transactionID++;
    if(sock->transactionID==0xFF)
      sock->transactionID=0x01;
  }

  /* Receive data */
  do
  {
    /* read packet */
    read = PL_SLPRead(socket,((UBYTE *)buffer)+offset,remain,&slp,&padp);
    if(read == -1)                        // Error occured
    {
      offset = -1;
      break;
    }

    /* check for plausibility */
    if(padp.type == PLPADP_TICKLE)        // Don't care about tickles
    {
      continue;
    }

    if(padp.type == PLPADP_ACK)           // Acknowledge
    {
      // No acknowledges should occur. Anyhow, if we get an acknowledge,
      // we will just ignore it.
      continue;
    }

    gotoffset = (padp.flags & PLPADPF_FIRST ? 0 : padp.size);

    if(   (padp.type != PLPADP_DATA)      // Bad packet?
       || (slp.transID != sock->transactionID)
       || (offset==0 && !(padp.flags&PLPADPF_FIRST))
       || (gotoffset != offset)
      )
    {
      sock->lastError = PLERR_BADPACKET;
      offset = -1;
#ifdef DEBUGOUT
  Printf("DEBUG: File %s Line %ld: Bad Packet\n",__FILE__,__LINE__);
#endif
      break;
    }

#ifdef PADPDEBUG
  Printf("PADP A<-P: type=%ld flags=0x%02lx size=%ld tID=0x%02lx\n",padp.type,padp.flags,padp.size,slp.transID);
#endif

    /* Send ACK */
    ackpadp.type   = PLPADP_ACK;
    ackpadp.flags  = padp.flags;
    ackpadp.size   = padp.size;
    ackslp.transID = sock->transactionID;
    if(-1 == PL_SLPWrite(socket,&ackpadp,sizeof(struct PL_PADP_Header),&ackslp,NULL))
    {
      offset = -1;
      break;
    }

#ifdef PADPDEBUG
  Printf("PADP A<-P: [sent ACK]\n");
#endif

    /* Next Packet */
    offset += read;
    remain -= read;

    if(padp.flags&PLPADPF_LAST) break;
  }
  while(remain>0);

  return(offset);                         // Packets received
}
//<


/********************************************************************/
