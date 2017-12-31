#ifndef FUSION_ENCODING_H
#define	FUSION_ENCODING_H


/*OPCode defines*/
#define OPC_R_TYPE	0x01



/*Macros for generating binary code of instruction*/

#define MAKE_R_TYPE(RD, RSA, RSB, SHFT, ALUOP) \
		(RD << 21) | (RSA << 16) | (RSB << 11) | (SHFT << 4)\
		| (ALUOP) | (OPC_R_TYPE << 26)
#define MAKE_I_TYPE(RD, RSA, IMM, ALUOP) \
		(RD <<21 ) | (RSA << 16) | (IMM << 4)\
		| (ALUOP) | (OPC_I_TYPE << 26)
#define MAKE_L_TYPE(RD, RSA, FUNCT, IMM) \
		(RD << 21) | (RSA << 16) | (FUNCT << 14)\
		| (IMM) | (OPC_L_TYPE << 26)
#define MAKE_LI_TYPE(RD, DSEL, IMM) \
		(RD << 21) | (DSEL << 16) | (IMM) | (OPC_LI_TYPE << 26)
#define MAKE_S_TYPE(FUNCT, IMM_HI, RSA, RSB, IMM_LO) \
		(FUNCT << 24) | (IMM_HI << 21) | (RSA << 16) | (RSB << 11)\
		| (IMM_LO) | (OPC_S_TYPE << 26)
#define MAKE_J_TYPE(IMM_HI, RSA, IMM_LO) \
		(IMM_HI << 21) | (RSA << 16) | (IMM_LO) | (OPC_J_TYPE << 26)
#define MAKE_B_TYPE(IMM_HI, RSA, RSB, IMM_LO, FUNCT) \
		(IMM_HI << 21) | (RSA << 16) | (RSB << 11) | (IMM_LO << 2)\
		| (FUNCT) | (OPC_B_TYPE << 26)
#define MAKE_SYS_TYPE(RD, RSA, FUNCT, IMM) \
		(RD << 21) | (RSA << 16) | (FUNCT << 8) | (IMM) | ( OPC_SYS_TYPE << 26)

