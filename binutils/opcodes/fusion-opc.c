/* fusion-opc.c -- Definitions for moxie opcodes.
   Copyright (C) 2009-2017 Free Software Foundation, Inc.
   Contributed by Dylan Wadler

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "opcode/fusion.h"
#include "opcode/fusion-opc.h"

const char* fusion_gpreg_name[32] ={
"zero",	"sp", "fp", "gp", "ra", "arg0", "arg1", "arg2", "arg3",
"rval0", "rval1", "gr0", "gr1", "gr2", "gr3", "gr4", "gr5", 
"gr6", "gr7", "gr8", "gr9", "gr10", "tmp0", "tmp1", "tmp2","tmp3",
"tmp4", "tmp5", "tmp6", "tmp7", "hi0", "low0"

};

const char* fusion_gpreg_num[32] ={
"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8",
"R9", "R10", "R11", "R12", "R13", "R14", "R15", "R16",
"R17", "R18", "R19", "R20", "R21", "R22", "R23", "R24",
"R25", "R26", "R27", "R28", "R29", "R30", "R31"

};

//more will be added in the future
const char* fusion_spreg_name[13] ={
	"CPUREV", "CPNUM", "CPO0", "n/a", "CPO0", "n/a", "CPO1",
	"n/a", "CPO3", "n/a", "STAT", "n/a", "OPCAR"
};




//Uses ALU OP field to determine
const fusion_opc_info_t fusion_insn_R[NUM_INSN_R] = {
/* Name  	ALUOP	args		frmt		cpid		imm_mask		opc	*/
 {"add", 	0x00,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"sub", 	0x01, 	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"addu",	0x02,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"subu",	0x03,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"tcmp",	0x04,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"and",	0x05,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"or",		0x06,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"xor",	0x07,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"sal",	0x08,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"sar",	0x09,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"sll",	0x0a,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"slr",	0x0b,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"comp",	0x0c,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"nri",	0x0d,	USE_NONE,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"nri",	0x0e,	USE_NONE,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"nri",	0x0f,	USE_NONE,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"nop",	0x10,	USE_NONE,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x00},
};

const fusion_opc_info_t fusion_insn_I[NUM_INSN_I] = {
/* Name  	ALUOP	args		frmt		cpid		imm_mask		opc	*/
{"addi",	0x00, 	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"subi",	0x01, 	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"addui",	0x02,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"subui",	0x03,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"nri",		0x04,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"andi",	0x05,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"ori",		0x06,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"xori",	0x07,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"sali",	0x08,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"sari",	0x09,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"slli",	0x0a,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"slri",	0x0b,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"compi",	0x0c,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"nri",		0x0d,	USE_NONE,	FRMT_I,		CPID_MAIN,	MASK_NO_IMM,	0x03},
{"nri",		0x0e,	USE_NONE,	FRMT_I,		CPID_MAIN,	MASK_NO_IMM,	0x03},
{"nri",		0x0f,	USE_NONE,	FRMT_I,		CPID_MAIN,	MASK_NO_IMM,	0x03}
};
const fusion_opc_info_t fusion_insn_L[NUM_INSN_L] = {
/* Name  	Funct	args		frmt		cpid		imm_mask		opc	*/
{"lw",		0x0, 	USE_RDAI,	FRMT_L,		CPID_MAIN,	MASK_IMM_L,		0x12},
{"lh",		0x1,	USE_RDAI,	FRMT_L,		CPID_MAIN,	MASK_IMM_L,		0x12},
{"lth",		0x2,	USE_RDAI,	FRMT_L,		CPID_MAIN,	MASK_IMM_L,		0x12},
{"lb",		0x3,	USE_RDAI,	FRMT_L,		CPID_MAIN,	MASK_IMM_L,		0x12}

};
const fusion_opc_info_t fusion_insn_LI[NUM_INSN_LI] = {
/* Name  	DSEL	args		frmt		cpid		imm_mask		opc	*/
{"li",		0x0,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lsi",		0x1,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lgi",		0x2,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lui",		0x3,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lusi",	0x4,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lugi",	0x5,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"nri",		0x6,	USE_NONE,	FRMT_LI,	CPID_MAIN,	MASK_NO_IMM,	0x02},
{"nri",		0x7,	USE_NONE,	FRMT_LI,	CPID_MAIN,	MASK_NO_IMM,	0x02},
{"lni",		0x8,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lnsi",	0x9,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lngi",	0xa,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"luni",	0xb,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lunsi",	0xc,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lungi",	0xd,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"nri",		0xe,	USE_NONE,	FRMT_LI,	CPID_MAIN,	MASK_NO_IMM,	0x02},
{"nri",		0xf,	USE_NONE,	FRMT_LI,	CPID_MAIN,	MASK_NO_IMM,	0x02}
};
const fusion_opc_info_t fusion_insn_S[NUM_INSN_S] = {
/* Name  	Funct	args		frmt		cpid		imm_mask		opc	*/
{"sw",		0x0, 	USE_RABI,	FRMT_S,		CPID_MAIN,	MASK_IMM_S,		0x3a},
{"sh",		0x1,	USE_RABI,	FRMT_S,		CPID_MAIN,	MASK_IMM_S,		0x3a},
{"sth",		0x2,	USE_RABI,	FRMT_S,		CPID_MAIN,	MASK_IMM_S,		0x3a},
{"sb",		0x3,	USE_RABI,	FRMT_S,		CPID_MAIN,	MASK_IMM_S,		0x3a}
};
const fusion_opc_info_t fusion_insn_J[NUM_INSN_J] = {
/* Name  	USESREG	args		frmt		cpid		imm_mask		opc	*/
{"j",		0x0,	USE_I,		FRMT_J,		CPID_MAIN,	MASK_IMM_J,		0x06},
{"jr",		0x1,	USE_RAI,	FRMT_J,		CPID_MAIN,	MASK_IMM_J,		0x06}
};
const fusion_opc_info_t fusion_insn_JL[NUM_INSN_JL] = {
/* Name  	USESREG	args		frmt		cpid		imm_mask		opc	*/
{"jal",		0x0,	USE_I,		FRMT_J,		CPID_MAIN,	MASK_IMM_J,		0x07},
{"jrl",		0x1,	USE_RAI,	FRMT_J,		CPID_MAIN,	MASK_IMM_J,		0x07}
};
const fusion_opc_info_t fusion_insn_B[NUM_INSN_B] = {
/* Name  	Funct	args		frmt		cpid		imm_mask		opc	*/
{"beq",		0x0,	USE_RABI,	FRMT_B,		CPID_MAIN,	MASK_IMM_B,		0x05},
{"bne",		0x1,	USE_RABI,	FRMT_B,		CPID_MAIN,	MASK_IMM_B,		0x05},
{"bgt",		0x2,	USE_RABI,	FRMT_B,		CPID_MAIN,	MASK_IMM_B,		0x05},
{"blt",		0x3,	USE_RABI,	FRMT_B,		CPID_MAIN,	MASK_IMM_B,		0x05}

};
const fusion_opc_info_t fusion_insn_SYS[NUM_INSN_SYS] = {
/* Name  	Funct	args		frmt		cpid		imm_mask		opc	*/
{"syscall",	0x00,	USE_RDAI,	FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{"sysret",	0x01,	USE_RDAI,	FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{"stspr",	0x02,	USE_RDA,	FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{"ldspr",	0x03,	USE_RDA,	FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{"sync",	0x04,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"pmir",	0x05,	USE_I,		FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{"pmd",		0x06, 	USE_I,		FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{"nri",		0x07,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x08,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x09,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x0a,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x0b,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x0c,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x0d,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x0e,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x0f,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x10,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x11,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x12,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x13,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x14,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x15,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x16,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x17,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x18,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x19,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x1a,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x1b,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x1c,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x1d,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x1e,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x1f,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x20,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x21,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x22,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x23,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x24,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x25,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x26,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x27,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x28,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x29,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x2a,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x2b,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x2c,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x2d,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x2e,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x2f,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x30,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x31,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x32,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x33,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x34,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x35,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x36,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x37,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x38,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x39,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x3a,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x3b,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x3c,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x3d,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x3e,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x3f,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x40,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x41,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x42,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x43,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x44,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x45,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x46,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x47,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x48,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x49,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x4a,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x4b,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x4c,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x4d,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x4e,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x4f,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x50,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x51,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x52,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x53,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x54,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x55,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x56,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x57,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x58,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x59,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x5a,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x5b,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x5c,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x5d,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x5e,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x5f,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x60,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x61,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x62,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x63,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x64,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x65,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x66,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x67,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x68,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x69,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x6a,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x6b,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x6c,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x6d,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x6e,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x6f,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x70,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x71,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x72,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x73,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x74,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x75,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x76,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x77,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x78,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x79,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x7a,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x7b,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x7c,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x7d,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x7e,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x7f,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x80,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x81,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x82,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x83,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x84,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x85,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x86,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x87,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x88,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x89,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x8a,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x8b,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x8c,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x8d,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x8e,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x8f,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x90,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x91,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x92,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x93,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x94,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x95,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x96,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x97,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x98,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x99,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x9a,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x9b,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x9c,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x9d,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x9e,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0x9f,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xa0,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xa1,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xa2,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xa3,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xa4,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xa5,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xa6,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xa7,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xa8,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xa9,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xaa,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xab,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xac,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xad,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xae,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xaf,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xb0,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xb1,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xb2,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xb3,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xb4,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xb5,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xb6,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xb7,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xb8,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xb9,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xba,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xbb,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xbc,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xbd,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xbe,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xbf,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xc0,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xc1,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xc2,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xc3,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xc4,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xc5,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xc6,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xc7,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xc8,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xc9,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xca,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xcb,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xcc,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xcd,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xce,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xcf,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xd0,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xd1,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xd2,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xd3,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xd4,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xd5,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xd6,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xd7,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xd8,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xd9,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xda,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xdb,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xdc,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xdd,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xde,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xdf,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xe0,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xe1,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xe2,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xe3,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xe4,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xe5,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xe6,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xe7,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xe8,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xe9,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xea,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xeb,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xec,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xed,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xee,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xef,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xf0,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xf1,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xf2,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xf3,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xf4,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xf5,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xf6,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xf7,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xf8,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xf9,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xfa,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xfb,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xfc,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xfd,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xfe,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"nri",		0xff,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08}
};




const fusion_opc_info_t fusion_insn_all[NUM_INSN] = {
 /*no op*/
 {"nop",	0x00,	USE_NONE,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x00},
/*		Integer Instructions									*/
/* Name  	ALUOP	args		frmt		cpid		imm_mask		opc	*/
 {"add", 	0x00,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"sub", 	0x01, 	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"addu",	0x02,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"subu",	0x03,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"tcmp",	0x04,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"and",	0x05,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"or",		0x06,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"xor",	0x07,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"sal",	0x08,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"sar",	0x09,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"sll",	0x0a,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"slr",	0x0b,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"comp",	0x0c,	USE_RDAB,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
 {"nri",	0x0d,	USE_NONE,	FRMT_R,		CPID_MAIN,	MASK_NO_IMM,	0x01},
/*		Immediate Instructions									*/
/* Name  	ALUOP	args		frmt		cpid		imm_mask		opc	*/
{"addi",	0x00, 	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"subi",	0x01, 	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"addui",	0x02,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"subui",	0x03,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"andi",	0x05,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"ori",		0x06,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"xori",	0x07,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"sali",	0x08,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"sari",	0x09,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"slli",	0x0a,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"slri",	0x0b,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
{"compi",	0x0c,	USE_RDAI,	FRMT_I,		CPID_MAIN,	MASK_IMM_I,		0x03},
/*		Load Instructions									*/
/* Name  	Funct	args		frmt		cpid		imm_mask		opc	*/
{"lw",		0x0, 	USE_RDAI,	FRMT_L,		CPID_MAIN,	MASK_IMM_L,		0x12},
{"lh",		0x1,	USE_RDAI,	FRMT_L,		CPID_MAIN,	MASK_IMM_L,		0x12},
{"lth",		0x2,	USE_RDAI,	FRMT_L,		CPID_MAIN,	MASK_IMM_L,		0x12},
{"lb",		0x3,	USE_RDAI,	FRMT_L,		CPID_MAIN,	MASK_IMM_L,		0x12},
/*		Load Immediate Instructions								*/
/* Name  	DSEL	args		frmt		cpid		imm_mask		opc	*/
{"li",		0x0,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lsi",		0x1,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lgi",		0x2,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lui",		0x3,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lusi",	0x4,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lugi",	0x5,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lni",		0x8,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lnsi",	0x9,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lngi",	0xa,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"luni",	0xb,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lunsi",	0xc,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
{"lungi",	0xd,	USE_RDI,	FRMT_LI,	CPID_MAIN,	MASK_IMM_LI,		0x02},
/*		Store Instructions									*/
/* Name  	Funct	args		frmt		cpid		imm_mask		opc	*/
{"sw",		0x0, 	USE_RABI,	FRMT_S,		CPID_MAIN,	MASK_IMM_S,		0x3a},
{"sh",		0x1,	USE_RABI,	FRMT_S,		CPID_MAIN,	MASK_IMM_S,		0x3a},
{"sth",		0x2,	USE_RABI,	FRMT_S,		CPID_MAIN,	MASK_IMM_S,		0x3a},
{"sb",		0x3,	USE_RABI,	FRMT_S,		CPID_MAIN,	MASK_IMM_S,		0x3a},
/*		Jump Instructions									*/
/* Name  	USESREG	args		frmt		cpid		imm_mask		opc	*/
{"j",		0x0,	USE_RAI,	FRMT_J,		CPID_MAIN,	MASK_IMM_J,		0x06},
{"jr",		0x1,	USE_RAI,	FRMT_J,		CPID_MAIN,	MASK_IMM_J,		0x06},
/*		Jump Link Instructions									*/
/* Name  	USESREG	args		frmt		cpid		imm_mask		opc	*/
{"jal",		0x0,	USE_RAI,	FRMT_J,		CPID_MAIN,	MASK_IMM_J,		0x07},
{"jrl",		0x1,	USE_RAI,	FRMT_J,		CPID_MAIN,	MASK_IMM_J,		0x07},
/*		Branch Instructions									*/
/* Name  	Funct	args		frmt		cpid		imm_mask		opc	*/
{"beq",		0x0,	USE_RABI,	FRMT_B,		CPID_MAIN,	MASK_IMM_B,		0x05},
{"bne",		0x1,	USE_RABI,	FRMT_B,		CPID_MAIN,	MASK_IMM_B,		0x05},
{"bgt",		0x2,	USE_RABI,	FRMT_B,		CPID_MAIN,	MASK_IMM_B,		0x05},
{"blt",		0x3,	USE_RABI,	FRMT_B,		CPID_MAIN,	MASK_IMM_B,		0x05},
/*		System Instructions									*/
/* Name  	Funct	args		frmt		cpid		imm_mask		opc	*/
{"syscall",	0x00,	USE_RDAI,	FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{"sysret",	0x01,	USE_RDAI,	FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{"stspr",	0x02,	USE_RDA,	FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{"ldspr",	0x03,	USE_RDA,	FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{"sync",	0x04,	USE_NONE,	FRMT_SYS,	CPID_MAIN,	MASK_NO_IMM,		0x08},
{"pmir",	0x05,	USE_I,		FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{"pmd",		0x06, 	USE_I,		FRMT_SYS,	CPID_MAIN,	MASK_IMM_SYS,	0x08},
{0,			0,		0,			0,			0,			0,				0   }
};
