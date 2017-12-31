#ifndef FUSION_ENCODING_H
#define	FUSION_ENCODING_H


/*OPCode defines*/
#define OPC_R_TYPE	0x01



/*Macros for generating binary code of instruction*/

#define MAKE_R_TYPE(RD, RSA, RSB, SHFT, ALUOP) \
		(RD << 21) | (RSA << 16) | (RSB << 11) | (SHFT << 4)\
		| (ALUOP) | ( OPC_R_TYPE << 26)
