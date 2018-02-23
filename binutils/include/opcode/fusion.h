/* Definitions for decoding the moxie opcode table.
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

#ifndef FUSION_OPC_H
#define FUSION_OPC_H

#include <stdint.h>

/* Operand Type Definitions*/
#define USE_NONE			0	//Doesn't have use, just for system instructions
#define USE_RDAB			1	//Uses Rd, RSa, RSb
#define USE_RDAI			2	//Uses Rd, RSa, Immediate
#define USE_RDI				3	//Uses Rd, Immediate
#define USE_RABI			4	//Uses RSa, RSb, Immediate
#define USE_RAI				5	//Uses RSa, Immediate
#define USE_I				6	//Uses Immediate only
#define USE_RDA				7	//Uses Rd, RSa
#define USE_RAB				8	//Uses RSa, RSb
#define USE_RABI_O		9	//Uses RSa, RSb, immediate, with rsb
								//as offset value
#define USE_RDAI_O		10  //Same as USE_RDAI, but with RSa
								//as offset value
#define USE_RAI_O		11  //Same as USE_RAI, but with RSa
								//as offset value							//
#define MAX_USE_OP		12

/*Defines for number of current implemented instructions*/
#define NUM_INSN_R			17 // 16 total instructions,only 12 at
					   //the moment; include nop to 16 
					   //other instructions
					   
#define NUM_INSN_I			16 // 16 total instructions, only 12 a the moment
#define NUM_INSN_L			4
#define NUM_INSN_LI			32 //32 total instructions, only 6 at the moment
#define NUM_INSN_S			4
#define NUM_INSN_J			2
#define NUM_INSN_JL			2
#define NUM_INSN_B			4
#define NUM_INSN_SYS		256 //256 total instructions, only 7 at the moment
#define NUM_INSN		( NUM_INSN_R + NUM_INSN_I + NUM_INSN_L\
						+ NUM_INSN_LI + NUM_INSN_S + NUM_INSN_J\
						+ NUM_INSN_JL + NUM_INSN_SYS )

/**** Bit field Masks ****/
#define MASK_RD				0x03e00000
#define MASK_RSA			0x001f0000
#define MASK_RSB			0x0000f800
#define MASK_SHFT			0x000007f0	
#define MASK_ALUOP			0x0000000f
#define MASK_IMM_I			0x0000fff0
#define MASK_IMM_L			0x00003fff
#define MASK_FUNCT_L		0x0000c000
#define MASK_DSEL_LI		0x001e0000
#define MASK_IMM_LI			0x0000ffff
#define MASK_FUNCT_S		0x0f000000
#define MASK_IMM_S			0x00e007ff
#define MASK_IMM_HI_S		0x00e00000
#define MASK_IMM_LO_S		0x000007ff
#define MASK_IMM_J			0x03e0ffff
#define MASK_IMM_HI_J		0x03e00000	
#define MASK_IMM_LO_J		0x0000ffff
#define MASK_IMM_B			0x03e007fc
#define MASK_IMM_HI_B		0x03e00000
#define MASK_IMM_LO_B		0x000007fc
#define MASK_FUNCT_B		0x00000003
#define MASK_FUNCT_SYS		0x0000ff00
#define MASK_IMM_SYS		0x000000ff

/* Bit field split mask for immediates*/
#define SPLIT_S_IMM_LO		0x000007ff //lower byte
#define SPLIT_S_IMM_HI		0x00003800 
#define SPLIT_J_IMM_LO		0x0000ffff //lower 16 bits
#define SPLIT_J_IMM_HI		0x001f0000
#define SPLIT_B_IMM_LO		0x000001ff //lower byte
#define SPLIT_B_IMM_HI		0x00003e00 

/* Bit shift amounts*/
#define SHFT_IMM_HI_S		10 //11 -> 21
#define SHFT_IMM_HI_J		5 //16 ->21
#define SHFT_IMM_HI_B		12 //9->21
#define SHFT_IMM_LO_B		2 //0 -> 2

/* Bit Field Macros */
#define GET_RD(i)			(MASK_RD & i) >> 21
#define GET_RSA(i)			(MASK_RSA & i) >> 16
#define GET_RSB(i)			(MASK_RSB & i) >> 11
#define GET_SHFT(i)			(MASK_SHFT & i) >> 4	
#define GET_ALUOP(i)		(MASK_ALUOP & i)
#define GET_IMM_I(i)		(MASK_IMM_I & i) >> 4
#define GET_IMM_L(i)		(MASK_IMM_L & i)
#define GET_FUNCT_L(i)		(MASK_FUNCT_L & i) >> 14
#define GET_DSEL_LI(i)		(MASK_DSEL_LI & i) >> 17
#define GET_IMM_LI(i)		(MASK_IMM_LI & i)
#define GET_FUNCT_S(i)		( (MASK_FUNCT_S & i) >> 24 ) & 0x3
#define GET_IMM_S(i)		(MASK_IMM_LO_S & i) | ( (MASK_IMM_HI_S & i) >> 10)
#define GET_IMM_J(i)		(MASK_IMM_LO_J & i) | ( (MASK_IMM_HI_J & i) >> SHFT_IMM_HI_J)	
#define GET_IMM_B(i)		( (MASK_IMM_LO_B & i) >> SHFT_IMM_LO_B ) | ( (MASK_IMM_HI_B & i) >> SHFT_IMM_HI_B ) 
#define GET_FUNCT_B(i)		(MASK_FUNCT_B & i)
#define GET_FUNCT_SYS(i)	(MASK_FUNCT_SYS & i) >> 8
#define GET_IMM_SYS(i)		(MASK_IMM_SYS & i)

/*Opcode related defines*/
#define MASK_OPC			0xfc000000 //upper 6 bits gives opcode
#define MASK_R_OPC			OPC_INT
#define MASK_LI_OPC			OPC_LI
#define MASK_IMM_OPC		OPC_IMM
#define MASK_B_OPC			OPC_BRANCH
#define MASK_JMP_OPC		OPC_JMP
#define MASK_JLNK_OPC		OPC_JLNK
#define MASK_RMEM_OPC		OPC_LD
#define MASK_WMEM_OPC		OPC_ST
#define MASK_SYS_OPC		OPC_SYS //more system instructions may change this
#define MASK_MSB_OPC		0x11000000	//31st bit of instruction
#define GET_OPC(i)			(MASK_OPC & i) >>26  //shift to make upper 6 bits, fit in 1 byte
#define MASK_NO_IMM			0x00000000

/*OPCode match macros*/
#define IS_CP_INSN(i)	(GET_OPC(i) == MASK_MSB_OPC) 	//is coprocessor instruction if first bit of opcode is 1
#define IS_R_INSN(i)	(GET_OPC(i) == MASK_R_OPC)  			//returns 1 if match, otherwise 0
#define IS_LI_INSN(i)	(GET_OPC(i) == MASK_LI_OPC)
#define IS_IMM_INSN(i)	(GET_OPC(i) == MASK_IMM_OPC)
#define IS_B_INSN(i)	(GET_OPC(i) == MASK_B_OPC)
#define IS_JMP_INSN(i)	(GET_OPC(i) == MASK_JMP_OPC)
#define IS_JLNK_INSN(i)	(GET_OPC(i) == MASK_JLNK_OPC)
#define IS_RMEM_INSN(i)	(GET_OPC(i) == MASK_RMEM_OPC)
#define IS_WMEM_INSN(i)	(GET_OPC(i) == MASK_WMEM_OPC)
#define IS_SYS_INSN(i)	(GET_OPC(i) == MASK_SYS_OPC)

/*Determining format Macros*/
#define IS_R_TYPE(i) 	( IS_R_INSN(i) )	
#define IS_I_TYPE(i) 	( IS_IMM_INSN(i) ) 
#define IS_L_TYPE(i) 	( IS_RMEM_INSN(i)  )
#define IS_LI_TYPE(i) 	( IS_LI_INSN(i) )
#define IS_S_TYPE(i) 	( IS_WMEM_INSN(i) )
#define IS_J_TYPE(i) 	( IS_JMP_INSN(i) )
#define IS_JL_TYPE(i)   ( IS_JLNK_INSN(i) )
#define IS_B_TYPE(i) 	( IS_B_INSN(i) )
#define IS_SYS_TYPE(i) 	( IS_SYS_INSN(i) )


/*Defines for Instruction formats*/
#define FRMT_R				0
#define FRMT_I				1
#define FRMT_L				2
#define FRMT_LI				3
#define FRMT_S				4
#define FRMT_J				5
#define FRMT_B				6
#define FRMT_SYS			7
#define FRMT_MAX			9

/*Defines for Co-Processor ID*/
#define CPID_MAIN		0
#define CPID_MACRO		1
#define CPID_MAX		2

/*Defines for Opcodes*/
#define OPC_INT			0x13 //010011
#define OPC_IMM			0x16 //010110
#define OPC_LD			0x1e //011110
#define OPC_LI			0x10 //010000
#define OPC_ST			0x1d //011101
#define OPC_JMP			0x0c //001100	
#define OPC_JLNK		0x04 //000100
#define OPC_BRANCH		0x0d //001101
#define OPC_SYS			0x20 //100000



/*Instruction bits*/
typedef uint32_t insn_t;


typedef struct fusion_opc_info_t {
	const char* name;		//instruction name
	insn_t index;			//how instruction is determined
	unsigned args;			//which operands for instruction
	unsigned frmt;			//instruction format type
	unsigned cpid;			//ID for coprocessor; 0 is main
	insn_t imm_mask;		//mask for generating immediate bit position; 0 if no immediate
	insn_t opc;			//the actual opcode

} fusion_opc_info_t;

/*Macro IDs*/
enum{
	M_LA,
	M_CALL,
	M_RET,
	M_MAX

};


extern const fusion_opc_info_t fusion_insn_all[NUM_INSN];
extern const fusion_opc_info_t fusion_insn_R[NUM_INSN_R]; 
extern const fusion_opc_info_t fusion_insn_I[NUM_INSN_I];
extern const fusion_opc_info_t fusion_insn_L[NUM_INSN_L];
extern const fusion_opc_info_t fusion_insn_LI[NUM_INSN_LI]; 
extern const fusion_opc_info_t fusion_insn_S[NUM_INSN_S]; 
extern const fusion_opc_info_t fusion_insn_J[NUM_INSN_J];
extern const fusion_opc_info_t fusion_insn_JL[NUM_INSN_JL];
extern const fusion_opc_info_t fusion_insn_B[NUM_INSN_B];
extern const fusion_opc_info_t fusion_insn_SYS[NUM_INSN_SYS]; 

#endif
