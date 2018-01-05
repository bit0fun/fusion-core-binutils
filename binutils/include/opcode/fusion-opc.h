#ifndef FUSION_ENCODING_H
#define	FUSION_ENCODING_H

#include "fusion.h"
/*Macros for generating binary code of instruction*/

#define MAKE_R_TYPE(RD, RSA, RSB, SHFT, ALUOP) \
		( ( RD & 0x1f ) << 21) | ( (RSA  & 0x1f )<< 16) \
		| ( (RSB & 0x3f ) << 11) | ( (SHFT << 4) & 0x7f )\
		| ( ALUOP & 0x0f ) | ( ( OPC_INT & 0x3f ) << 26)

#define MAKE_I_TYPE(RD, RSA, IMM, ALUOP) \
		( ( RD & 0x1f ) <<21 ) | ( ( RSA & 0x1f ) << 16) \
       		| ( ( IMM & 0xfff )  << 4)| ( ALUOP & 0x0f ) \
		| ( ( OPC_IMM & 0x3f ) << 26)

#define MAKE_L_TYPE(RD, RSA, FUNCT, IMM) \
		( ( RD & 0x1f ) << 21) | ( ( RSA & 0x1f ) << 16) \
       		| ( ( FUNCT & 0x3 ) << 14) | ( IMM & 0x3fff ) \
		| ( ( OPC_LD & 0x3f ) << 26)

#define MAKE_LI_TYPE(RD, DSEL, IMM) \
		( ( RD & 0x1f ) << 21) | ( ( DSEL & 0xf )<< 16) \
		| (IMM & 0xffff) | ( ( OPC_LI & 0x3f ) << 26)

#define MAKE_S_TYPE(FUNCT, RSA, RSB, IMM) \
		( ( FUNCT & 0x3 ) << 24) | GET_S_IMM(IMM) \
		| ( ( RSA & 0x1f ) << 16) | ( ( RSB & 0x1f ) << 11) \
		| GET_S_IMM(IMM) | ( ( OPC_ST & 0x3f ) << 26)

#define MAKE_J_TYPE( RSA, IMM) \
		( ( RSA & 0x1f ) << 16) | GET_J_IMM(IMM) \
		| ( ( OPC_JMP & 0x3f ) << 26)

#define MAKE_JL_TYPE( RSA, IMM) \
		 ( ( RSA & 0x1f ) << 16) | GET_J_IMM(IMM) \
		| ( ( OPC_JLNK & 0x3f ) << 26)

#define MAKE_B_TYPE( RSA, RSB, IMM, FUNCT) \
		  ( ( RSA & 0x1f ) << 16) | ( ( RSB & 0x1f ) << 11) \
		| GET_B_IMM(IMM) | ( FUNCT & 0x3) \
		| ( (OPC_BRANCH & 0x3f ) << 26)

#define MAKE_SYS_TYPE(RD, RSA, FUNCT, IMM) \
		  ( (RD & 0x1f ) << 21) | ( (RSA & 0x1f) << 16) \
		| ( (FUNCT & 0xff) << 8) | ( IMM & 0xff ) \
		| ( ( OPC_SYS & 0x3f ) << 26)

/* Macros for Splitting up Immediate Values*/

#define GET_SPLIT_S_IMM_HI(IMM) 	( IMM & SPLIT_S_IMM_HI ) << SHFT_IMM_HI_S
#define GET_SPLIT_S_IMM_LO(IMM) 	( IMM & SPLIT_S_IMM_LO )
#define GET_SPLIT_J_IMM_HI(IMM) 	( IMM & SPLIT_J_IMM_HI ) << SHFT_IMM_HI_J
#define GET_SPLIT_J_IMM_LO(IMM) 	( IMM & SPLIT_J_IMM_LO )
#define GET_SPLIT_B_IMM_HI(IMM) 	( IMM & SPLIT_B_IMM_HI ) << SHFT_IMM_HI_B
#define GET_SPLIT_B_IMM_LO(IMM) 	( IMM & SPLIT_B_IMM_LO ) << SHFT_IMM_LO_B

#define GET_S_IMM(IMM)		( GET_SPLIT_S_IMM_HI(IMM) | GET_SPLIT_S_IMM_LO(IMM) )
#define GET_J_IMM(IMM)		( GET_SPLIT_J_IMM_HI(IMM) | GET_SPLIT_J_IMM_LO(IMM) )
#define GET_B_IMM(IMM)		( GET_SPLIT_B_IMM_HI(IMM) | GET_SPLIT_B_IMM_LO(IMM) )


#endif


