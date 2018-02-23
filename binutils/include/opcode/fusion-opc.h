#ifndef FUSION_ENCODING_H
#define	FUSION_ENCODING_H

#include "fusion.h"
/*Macros for generating binary code of instruction*/

#define MAKE_R_TYPE(RD, RSA, RSB, SHFT, ALUOP) \
		( ( ( RD & 0x1f ) << 21) | ( (RSA  & 0x1f )<< 16) \
		| ( (RSB & 0x3f ) << 11) | ( (SHFT << 4) & 0x7f )\
		| ( ALUOP & 0x0f ) | ( ( OPC_INT & 0x3f ) << 26) )

#define MAKE_I_TYPE(RD, RSA, IMM, ALUOP) \
		( ( ( RD & 0x1f ) <<21 ) | ( ( RSA & 0x1f ) << 16) \
       		| ( GEN_I_IMM(IMM) ) | ( ALUOP & 0x0f ) \
		| ( ( OPC_IMM & 0x3f ) << 26) )

#define MAKE_L_TYPE(RD, RSA, FUNCT, IMM) \
		( ( ( RD & 0x1f ) << 21) | ( ( RSA & 0x1f ) << 16) \
       		| ( ( FUNCT & 0x3 ) << 14) | ( IMM & 0x3fff ) \
		| ( ( OPC_LD & 0x3f ) << 26) )

#define MAKE_LI_TYPE(RD, DSEL, IMM) \
		( ( ( RD & 0x1f ) << 21) | ( ( DSEL & 0xf )<< 17) \
		| (IMM & 0xffff) | ( ( OPC_LI & 0x3f ) << 26) )

#define MAKE_S_TYPE(FUNCT, RSA, RSB, IMM) \
		( ( ( FUNCT & 0x3 ) << 24) | GEN_S_IMM(IMM) \
		| ( ( RSA & 0x1f ) << 16) | ( ( RSB & 0x1f ) << 11) \
		| ( ( OPC_ST & 0x3f ) << 26) )

#define MAKE_J_TYPE( RSA, IMM) \
		( GEN_RSA(RSA) | GEN_J_IMM(IMM) \
		| ( ( OPC_JMP & 0x3f ) << 26) )

#define MAKE_JL_TYPE( RSA, IMM) \
		(  ( ( RSA & 0x1f ) << 16) | GEN_J_IMM(IMM) \
		| ( ( OPC_JLNK & 0x3f ) << 26) )

#define MAKE_B_TYPE( RSA, RSB, IMM, FUNCT) \
		(  ( ( RSA & 0x1f ) << 16) | ( ( RSB & 0x1f ) << 11) \
		| GEN_B_IMM(IMM) | ( FUNCT & 0x3) \
		| ( (OPC_BRANCH & 0x3f ) << 26) )

#define MAKE_SYS_TYPE(RD, RSA, FUNCT, IMM) \
		(  ( (RD & 0x1f ) << 21) | ( (RSA & 0x1f) << 16) \
		| ( (FUNCT & 0xff) << 8) | GEN_SYS_IMM(IMM) \
		| ( ( OPC_SYS & 0x3f ) << 26) )

/* Macros for Splitting up Immediate Values*/

#define GET_SPLIT_S_IMM_HI(IMM) 	( IMM & SPLIT_S_IMM_HI ) << SHFT_IMM_HI_S
#define GET_SPLIT_S_IMM_LO(IMM) 	( IMM & SPLIT_S_IMM_LO )
#define GET_SPLIT_J_IMM_HI(IMM) 	( IMM & SPLIT_J_IMM_HI ) << SHFT_IMM_HI_J
#define GET_SPLIT_J_IMM_LO(IMM) 	( IMM & SPLIT_J_IMM_LO )
#define GET_SPLIT_B_IMM_HI(IMM) 	( IMM & SPLIT_B_IMM_HI ) << SHFT_IMM_HI_B
#define GET_SPLIT_B_IMM_LO(IMM) 	( IMM & SPLIT_B_IMM_LO ) << SHFT_IMM_LO_B

#define GEN_OPC(OPC)		( (OPC	 << 26 ) & 0xfc000000 )
#define GEN_RD(RD)			( ( RD   << 21 ) & MASK_RD )
#define GEN_RSA(RSA)		( ( RSA  << 16 ) & MASK_RSA )
#define GEN_RSB(RSB)		( ( RSB  << 11 ) & MASK_RSB )
#define GEN_SHFT(SHFT) 		( ( SHFT << 4  ) & MASK_SHFT )
#define GEN_ALUOP(OP)		( GET_ALUOP(OP) )
#define GEN_FUNCT_L(F)		( ( F & 0x03 ) << 14 )
#define GEN_FUNCT_S(F)		( ( F & 0x03 ) << 24 ) 
#define GEN_DSEL_LI(F)		( ( F & 0x0f ) << 17 )
#define GEN_I_IMM(IMM)		( ( IMM << 4 ) & MASK_IMM_I)  
#define GEN_S_IMM(IMM)		( ( GET_SPLIT_S_IMM_HI(IMM) | GET_SPLIT_S_IMM_LO(IMM) ) & MASK_IMM_S )
#define GEN_J_IMM(IMM)		( ( GET_SPLIT_J_IMM_HI(IMM) | GET_SPLIT_J_IMM_LO(IMM) ) & MASK_IMM_J )
#define GEN_B_IMM(IMM)		( ( GET_SPLIT_B_IMM_HI(IMM) | GET_SPLIT_B_IMM_LO(IMM) ) & MASK_IMM_B )
#define GEN_SYS_IMM(IMM)	( IMM & MASK_IMM_SYS)
#define GEN_L_IMM(IMM)		( GET_IMM_L(IMM) )
#define GEN_LI_IMM(IMM)		( GET_IMM_LI(IMM) )


#endif


