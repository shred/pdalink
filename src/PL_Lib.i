**
* PdaLink -- Connect Palm with Amiga
*
* Assembler headings
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
*


		IFND    PL_LIB_I
PL_LIB_I        SET     -1

		INCLUDE exec/libraries.i
		INCLUDE exec/lists.i
		INCLUDE exec/nodes.i


*--- REVISION CONTROL SYSTEM -------------------------------------------*

VERSION         EQU     2
REVISION        EQU     1
VSTRING         MACRO
		dc.b    'pdalink 2.1 (15.06.2004)',13,10,0
		ENDM
VERS            MACRO
		dc.b    'pdalink 2.1 (15.06.2004)'
		ENDM
PRGNAME         MACRO
		dc.b    'pdalink.library'
		ENDM

*--- REVISION CONTROL SYSTEM -------------------------------------------*

		rsreset                 ;PdaLink Base
plb_Library     rs.b    LIB_SIZE        ;Library-Node
plb_Flags       rs.w    1               ; Flags
plb_SysLib      rs.l    1               ; ^SysBase
plb_SegList     rs.l    1               ; ^SegBase
plb_SIZEOF      rs.w    0

PLLB_DELEXP     EQU     0               ;Delay expire
PLLF_DELEXP     EQU     1<<PLLB_DELEXP

		ENDC

*************************************************************************

