/* Disassemble fusion-core instructions.
   Copyright (C) 2009-2017 Free Software Foundation, Inc.

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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include <stdio.h>

#define STATIC_TABLE
#define DEFINE_TABLE

#include "opcode/fusion.h"
#include "disassemble.h"
#include "opintl.h"
#include "elf-bfd.h"

//extern const fusion_inst_t fusion_inst[128];

//extern static const char* fusion_gpr_names;

static fprintf_ftype fpr;
static void *stream;

/* Register definitions */

static const char* fusion_gpreg_name[32] ={
"zero",	"sp", "fp", "gp", "ra", "arg0", "arg1", "arg2", "arg3",
"rval0", "rval1", "gr0", "gr1", "gr2", "gr3", "gr4", "gr5", 
"gr6", "gr7", "gr8", "gr9", "gr10", "tmp0", "tmp1", "tmp2","tmp3",
"tmp4", "tmp5", "tmp6", "tmp7", "hi0", "low0"

};
/*
static const char* fusion_gpreg_num[32] ={
"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8",
"R9", "R10", "R11", "R12", "R13", "R14", "R15", "R16",
"R17", "R18", "R19", "R20", "R21", "R22", "R23", "R24",
"R25", "R26", "R27", "R28", "R29", "R30", "R31"

};
*/
//more will be added in the future
static const char* fusion_spreg_name[13] ={
	"CPUREV", "CPNUM", "CPO0", "n/a", "CPO0", "n/a", "CPO1",
	"n/a", "CPO3", "n/a", "STAT", "n/a", "OPCAR"
};


int print_insn_fusion (bfd_vma addr, struct disassemble_info *info) {

	int status;
	stream = info->stream;
	const fusion_insn_t *insn;
	unsigned short insn_word;
	fpr = info->fprintf_func;

	if ((status = info->read_memory_func (addr, (unsigned char*) &insn_word, 2, info)))
		goto fail;
//	fpr (stream, "%s", fusion_inst[opcode].name);
		
	//Figure out format
	if( IS_R_TYPE(insn_word) ) {	//need to get alu op to determine instruction
		insn = &fusion_insn_R[ GET_ALUOP(insn_word) ]; //which instruction to choose
		fpr(stream, "%s\t%s, %s, %s", insn->name,\
						fusion_gpreg_name[ GET_RD(insn_word) ],\
						fusion_gpreg_name[ GET_RSA(insn_word) ], \
						fusion_gpreg_name[GET_RSB(insn_word)] );
	} else if( IS_I_TYPE(insn_word) ) { //need to get alu op to determine instruction
		insn = &fusion_insn_I[ GET_ALUOP(insn_word) ];
		fpr(stream, "%s\t%s, %s, 0x%x", insn->name,\
						fusion_gpreg_name[ GET_RD(insn_word) ],\
						fusion_gpreg_name[ GET_RSA(insn_word) ],\
						GET_IMM_I(insn_word) );
				
	} else if( IS_L_TYPE(insn_word) ) {
		insn = &fusion_insn_L[ GET_FUNCT_L(insn_word) ];
		fpr(stream, "%s\t%s, 0x%x(%s)", insn->name, \
						fusion_gpreg_name[ GET_RD(insn_word) ],\
						GET_IMM_L(insn_word),\
						fusion_gpreg_name[ GET_RSA(insn_word) ] );
	
	} else if( IS_LI_TYPE(insn_word) ) {
		insn = &fusion_insn_LI[ GET_DSEL_LI(insn_word) ];
		fpr(stream, "%s\t%s, 0x%x", insn->name, \
						fusion_gpreg_name[ GET_RD(insn_word) ],\
						GET_IMM_L(insn_word) );
	
	} else if( IS_S_TYPE(insn_word) ) {
		insn = &fusion_insn_S[ GET_FUNCT_S(insn_word) ];
		fpr(stream, "%s\t%s, 0x%x(%s)", insn->name, \
						fusion_gpreg_name[ GET_RSB(insn_word) ],\
						GET_IMM_L(insn_word),\
						fusion_gpreg_name[ GET_RSA(insn_word) ] );
	
	} else if( IS_J_TYPE(insn_word) ) {
		if( GET_RSA(insn_word) == 0x00 ) { //if just a normal jump
			if ( IS_JLNK_INSN( insn_word ) ) //if jump and link
				fpr(stream, "jal\t0x%x",  GET_IMM_J(insn_word)  );
			else if (IS_JMP_INSN( insn_word ) )
				fpr(stream, "j\t0x%x",  GET_IMM_J(insn_word)  );
			else
				fpr(stream, "nri"); //not a real jump
		} else { //using register, no possible error here
			if ( IS_JLNK_INSN( insn_word ) ) //jump register and link
				fpr(stream, "jrl\t0x%x(%s)",  GET_IMM_J(insn_word), \
								fusion_gpreg_name[ GET_RSA(insn_word) ] );
			else if (IS_JMP_INSN( insn_word ) ) //jump register
				fpr(stream, "jr\t0%x(%s)", GET_IMM_J(insn_word), \
								fusion_gpreg_name[ GET_RSA(insn_word) ] );
			else
				fpr(stream, "nri"); //not a real jump
		}
	
	} else if( IS_B_TYPE(insn_word) ) {
			insn = &fusion_insn_B[ GET_FUNCT_B(insn_word) ];
			fpr(stream, "%s\t%s,%s,0x%x", insn->name,\
							fusion_gpreg_name[ GET_RSA(insn_word) ],\
							fusion_gpreg_name[ GET_RSB(insn_word) ],\
							GET_IMM_B(insn_word));
	
	} else if( IS_SYS_TYPE(insn_word) ) {
			insn = &fusion_insn_SYS[ GET_FUNCT_SYS(insn_word) ];
		switch(insn->insn_frmt) { //determine what operands, uses immediate	
			case USE_NONE: //no operands
				fpr(stream, "%s", insn->name);
				break;
			case USE_I: //only immediate
				fpr(stream, "%s\t0x%x", insn->name, GET_IMM_SYS(insn_word));
				break;
			case USE_RAI: //RSa and immediate
				fpr(stream, "%s\t%s,0x%x", insn->name,\
								fusion_gpreg_name[ GET_RSA(insn_word) ],\
								GET_IMM_SYS(insn_word));
				break;
			case USE_RDA:

				//check if instruction is accessing system registers
				if( GET_FUNCT_SYS(insn_word) == 0x02 )	{
					fpr(stream, "%s\t%s,%s", insn->name,\
							fusion_spreg_name[ GET_RD(insn_word) ],\
							fusion_gpreg_name[ GET_RSA(insn_word)] );
					break;
				} else if(GET_FUNCT_SYS(insn_word) == 0x03 ) {
					fpr(stream, "%s\t%s,%s", insn->name,\
							fusion_gpreg_name[ GET_RD(insn_word) ],\
							fusion_spreg_name[ GET_RSA(insn_word)] );
					break;
				} else {
				
					break;
				}
			case USE_RDI: //Rd and immediate
					fpr(stream, "%s\t%s,0x%x", insn->name,\
								fusion_gpreg_name[ GET_RD(insn_word) ],\
								 GET_IMM_SYS(insn_word)  );
				break;			
			case USE_RDAI: //Rd, RSa, and immediate
					fpr(stream, "%s\t%s, %s, 0x%x", insn->name,\
								fusion_gpreg_name[ GET_RD(insn_word) ],\
								fusion_gpreg_name[ GET_RSA(insn_word) ],\
								GET_IMM_SYS(insn_word) );
					break;
			default:
				fpr(stream, "nri"); //not a real instruction
				break;
		}
		
			
	
	} else if( IS_CP_INSN(insn_word) ) {
		fpr( stream, "cpinsn?"); //coprocessor instruction, should fix
								//this soon
	} else {
		fpr( stream, "nri"); //not real instruction, or just unknown
	}


	return 1;

	fail:
		info->memory_error_func (status, addr, info);
		return -1;
}
