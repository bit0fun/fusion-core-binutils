/* Definitions for decoding the Fusion-Core opcode table.
   Copyright (C) 2009-2017 Free Software Foundation, Inc.
   Contributed by Dylan Wadler.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#ifndef FUSION_ENCODING_H
#define	FUSION_ENCODING_H

#include "fusion.h"

/* Bitfield Generation Functions */

/* Get bitfileld Functions */
#define GET_OPC(i)			((MASK_OPC & i)			>> OFFSET_OPC) 
#define GET_RD(i)			((MASK_RD & i)			>> OFFSET_RD)
#define GET_RSA(i)			((MASK_RSA & i)			>> OFFSET_RSA)
#define GET_RSB(i)			((MASK_RSB & i)			>> OFFSET_RSB)
#define GET_SHFT(i)			((MASK_SHFT & i) 		>> OFFSET_SHFT)
#define GET_ALUOP(i)		((MASK_ALUOP & i) 		>> OFFSET_ALUOP)
#define GET_IMM_I(i)		((MASK_IMM_I & i) 		>> OFFSET_IMM_I)
#define GET_IMM_L(i)		((MASK_IMM_L & i)		>> OFFSET_IMM_L)
#define GET_FUNCT_L(i)		((MASK_FUNCT_L & i) 	>> OFFSET_FUNCT_L)
#define GET_DSEL_LI(i)		((MASK_DSEL_LI & i)		>> OFFSET_DSEL_LI)
#define GET_IMM_LI(i)		((MASK_IMM_LI & i)		>> OFFSET_IMM_LI)
#define GET_FUNCT_S(i)		(((MASK_FUNCT_S & i)	>> OFFSET_FUNCT_S ) & 0x3)
#define GET_IMM_S(i)		(((MASK_IMM_LO_S & i)	>> OFFSET_IMM_LO_S ) | ( (MASK_IMM_HI_S & i) >> OFFSET_IMM_HI_S ))
#define GET_IMM_J(i)		(((MASK_IMM_LO_J & i)	>> OFFSET_IMM_LO_J ) | ( (MASK_IMM_HI_J & i) >> OFFSET_IMM_HI_J))	
#define GET_IMM_B(i)		(((MASK_IMM_LO_B & i)	>> OFFSET_IMM_LO_B ) | ( (MASK_IMM_HI_B & i) >> OFFSET_IMM_HI_B ))
#define GET_FUNCT_B(i)		((MASK_FUNCT_B & i)		>> OFFSET_FUNCT_B)
#define GET_FUNCT_SYS(i)	((MASK_FUNCT_SYS & i)	>> OFFSET_FUNCT_SYS)
#define GET_IMM_SYS(i)		((MASK_IMM_SYS & i)		>> OFFSET_IMM_SYS)

/* Getting Split Immediate Values */
#define GET_HI_IMM_S(i)		( (i & SPLIT_S_IMM_HI ) << OFFSET_IMM_HI_S )
#define GET_LO_IMM_S(i)		( (i & SPLIT_S_IMM_LO ) << OFFSET_IMM_LO_S )
#define GET_HI_IMM_J(i)		( (i & SPLIT_J_IMM_HI ) << OFFSET_IMM_HI_J )
#define GET_LO_IMM_J(i)     ( (i & SPLIT_J_IMM_LO ) << OFFSET_IMM_LO_J )
#define GET_HI_IMM_B(i)		( (i & SPLIT_B_IMM_HI ) << OFFSET_IMM_HI_B )
#define GET_LO_IMM_B(i)     ( (i & SPLIT_B_IMM_LO ) << OFFSET_IMM_LO_B )


/* Generating bitfields */
#define GEN_OPC(OPC)		( ( (OPC) & GET_OPC(MASK_OPC) ) << OFFSET_OPC )
#define GEN_RD(RD)			( ( (RD) & GET_RD(MASK_RD) ) << OFFSET_RD )
#define GEN_RSA(RSA)		( ( (RSA) & GET_RSA(MASK_RSA) ) << OFFSET_RSA )
#define GEN_RSB(RSB)		( ( (RSB) & GET_RSB(MASK_RSB) ) << OFFSET_RSB )
#define GEN_SHFT(SHFT)		( ( (SHFT) & GET_SHFT(MASK_SHFT) ) << OFFSET_SHFT )
#define GEN_ALUOP(OP)		( ( (OP) & GET_ALUOP(MASK_ALUOP) ) << OFFSET_ALUOP )
#define GEN_FUNCT_L(F)		( ( (F) & GET_FUNCT_L(MASK_FUNCT_L) ) << OFFSET_FUNCT_L )
#define GEN_FUNCT_S(F)		( ( (F) & GET_FUNCT_S(MASK_FUNCT_S) ) << OFFSET_FUNCT_S )
#define GEN_DSEL_LI(F)		( ( (F) & GET_DSEL_LI(MASK_DSEL_LI) ) << OFFSET_DSEL_LI )
#define GEN_I_IMM(IMM)		( ( (IMM) & GET_IMM_I(MASK_IMM_I) ) << OFFSET_IMM_I )
#define GEN_S_IMM_HI(IMM)	( ( (IMM) & GET_HI_IMM_S(MASK_IMM_S) ) << OFFSET_IMM_HI_S )
#define GEN_S_IMM_LO(IMM)	( ( (IMM) & GET_LO_IMM_S(MASK_IMM_S) ) << OFFSET_IMM_LO_S )
#define GEN_S_IMM(IMM)		( GEN_S_IMM_HI(IMM) | GEN_S_IMM_LO(IMM) )
#define GEN_J_IMM_HI(IMM)	( ( (IMM) & GET_HI_IMM_J(MASK_IMM_J) ) << OFFSET_IMM_HI_J )
#define GEN_J_IMM_LO(IMM)   ( ( (IMM) & GET_LO_IMM_J(MASK_IMM_J) ) << OFFSET_IMM_LO_J )
#define GEN_J_IMM(IMM)		( GEN_J_IMM_HI(IMM) | GEN_J_IMM_LO(IMM) )
#define GEN_B_IMM_HI(IMM)	( ( (IMM) & GET_HI_IMM_S(MASK_IMM_B) ) << OFFSET_IMM_HI_B )
#define GEN_B_IMM_LO(IMM)   ( ( (IMM) & GET_LO_IMM_S(MASK_IMM_B) ) << OFFSET_IMM_LO_B )
#define GEN_B_IMM(IMM)		( GEN_B_IMM_HI(IMM) | GEN_B_IMM_LO(IMM) )
#define GEN_B_FUNCT(F)		( ( (F) & GET_FUNCT_B(MASK_FUNCT_B) ) << OFFSET_FUNCT_B )
#define GEN_SYS_FUNCT(F)	( ( (F) & GET_FUNCT_SYS(MASK_FUNCT_SYS) ) << OFFSET_FUNCT_SYS )
#define GEN_SYS_IMM(IMM)	( ( (IMM) & GET_IMM_SYS(MASK_IMM_SYS) ) << OFFSET_IMM_SYS )
#define GEN_L_IMM(IMM)		( ( (IMM) & GET_IMM_L(MASK_IMM_L) ) << OFFSET_IMM_L )
#define GEN_LI_IMM(IMM)		( ( (IMM) & GET_IMM_LI(MASK_IMM_LI) ) << OFFSET_IMM_LI )

/* Making Instruction Types */
#define MAKE_R_TYPE(RD, RSA, RSB, SHFT, ALUOP) \
		( GEN_RD( RD )  | GEN_RSA( RSA )  \
		| GEN_RSB( RSB ) | GEN_SHFT( SHFT )  \
		| GEN_ALUOP( ALUOP ) |  GEN_OPC( OPC_INT ) )

#define MAKE_I_TYPE(RD, RSA, IMM, ALUOP) \
		( GEN_RD( RD ) | GEN_RSA( RSA )  \
       	| GEN_I_IMM( IMM ) | GEN_ALUOP( ALUOP ) \
		| GEN_OPC( OPC_IMM ) )

#define MAKE_L_TYPE(RD, RSA, FUNCT, IMM) \
		( GEN_RD( RD ) | GEN_RSA( RSA ) \
		| GEN_FUNCT_L( FUNCT ) | GEN_L_IMM( IMM ) \
		| GEN_OPC( OPC_LD ) )

#define MAKE_LI_TYPE(RD, DSEL, IMM) \
		( GEN_RD( RD ) | GEN_DSEL_LI( DSEL ) \
		| GEN_LI_IMM( IMM ) | GEN_OPC( OPC_LI ) )

#define MAKE_S_TYPE(FUNCT, RSA, RSB, IMM) \
		( GEN_FUNCT_S( FUNCT ) | GEN_S_IMM(IMM) \
		| GEN_RSA( RSA ) | GEN_RSB( RSB ) \
		| GEN_OPC( OPC_ST ) )

#define MAKE_J_TYPE( RSA, IMM) \
		( GEN_RSA(RSA) | GEN_J_IMM(IMM) \
		| GEN_OPC( OPC_JMP ))

#define MAKE_JL_TYPE( RSA, IMM) \
		( GEN_RSA( RSA ) | GEN_J_IMM(IMM) \
		| GEN_OPC( OPC_JLNK ) )

#define MAKE_B_TYPE( RSA, RSB, IMM, FUNCT) \
		( GEN_RSA( RSA ) | GEN_RSB( RSB ) \
		| GEN_B_IMM(IMM) | GEN_B_FUNCT( FUNCT ) \
		| GEN_OPC( OPC_BRANCH ) )

#define MAKE_SYS_TYPE(RD, RSA, FUNCT, IMM) \
		( GEN_RD( RD ) | GEN_RSA( RSA ) \
		| GEN_SYS_FUNCT( FUNCT ) | GEN_SYS_IMM(IMM) \
		| GEN_OPC( OPC_SYS ) )

#endif


