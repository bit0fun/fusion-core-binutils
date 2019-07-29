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

#define SEXT_14B(N)			( ((N) & 0x3fff) | ( ( ((N) & 0x2000) ? 0xffffc000 : 0 ) ) )
#define SEXT_12B(N)			( ((N) & 0xfff ) | ( ( ((N) & 0x800 ) ? 0xfffff000 : 0 ) ) )
#define SEXT_21B(N)			( ((N) & 0x1fffff) | ( ( ((N) & 0x100000) ? 0xffe00000 : 0 ) ) )


//extern const fusion_inst_t fusion_inst[128];

//extern static const char* fusion_gpr_names;

static fprintf_ftype fpr;
static void *stream;

/* Register definitions */

extern const char* fusion_gpreg_name[32]; 
extern const char* fusion_gpreg_num[32];
extern const char* fusion_spreg_name[13];

int print_insn_fusion (bfd_vma addr, struct disassemble_info *info) {


	int status;
	stream = info->stream;
	const fusion_opc_info_t *insn;
	insn_t insn_word;
	fpr = info->fprintf_func;
	bfd_byte buf_insn[4];
	unsigned short opc;
	

	if ((status = info->read_memory_func (addr, buf_insn, 4, info)))
		goto fail;
	//in data section
	if( ( (unsigned long int) addr) > 0x2000000  ){
		fpr(stream, "ASCII: %c %c %c %c", (char)buf_insn[3], (char)buf_insn[2],\
										(char)buf_insn[1], (char)buf_insn[0]);
		return 4;
		
	}
	//insn_word = (insn_t) bfd_getb32(buf_insn);
	insn_word = (insn_t) bfd_getl32(buf_insn);
	opc = GET_OPC(insn_word);
	//Figure out format
	
	if( IS_R_TYPE(insn_word) ) {	//need to get alu op to determine instruction
		insn = &fusion_insn_R[ GET_ALUOP(insn_word) ]; //which instruction to choose
		fpr(stream, "%s\t%s,\t%s,\t%s", insn->name,\
						fusion_gpreg_name[ GET_RD(insn_word) ],\
						fusion_gpreg_name[ GET_RSA(insn_word) ], \
						fusion_gpreg_name[GET_RSB(insn_word)] );
	} else if( IS_I_TYPE(insn_word) ) { //need to get alu op to determine instruction
		insn = &fusion_insn_I[ GET_ALUOP(insn_word) ];
		insn_t aluop = insn->index;
		/*Determine to print out signed or unsigned number*/
		switch(aluop){
			/*Signed output*/
			case 0: //addi
			case 1: //subi
			case 8: //sali
			case 9: //sari
			case 10: //slli
			case 11: //slri
			case 12: //compi
				fpr(stream, "%s\t%s,\t%s,\t%d", insn->name,\
					fusion_gpreg_name[ GET_RD(insn_word) ],\
					fusion_gpreg_name[ GET_RSA(insn_word) ],\
					SEXT_12B(GET_IMM_I(insn_word)) );
				break;
			/*Unsigned output*/
			case 2: //addui
			case 3: //subui
			case 5: //andi
			case 6: //ori
			case 7: //xori
				fpr(stream, "%s\t%s,\t%s,\t0x%03x", insn->name,\
					fusion_gpreg_name[ GET_RD(insn_word) ],\
					fusion_gpreg_name[ GET_RSA(insn_word) ],\
					GET_IMM_I(insn_word) );
				break;
			//this is a special one, since we don't care about another operand
			case 4: //noti
				fpr(stream, "%s\t%s,\t0x%03x", insn->name,\
					fusion_gpreg_name[ GET_RD(insn_word) ],\
					GET_IMM_I(insn_word) );
				break;
			default:
			fpr(stream, "nri\t%s,\t%s,\t0x%03x", \
				fusion_gpreg_name[ GET_RD(insn_word) ],\
				fusion_gpreg_name[ GET_RSA(insn_word) ],\
				GET_IMM_I(insn_word) );
				break;
		}

	} else if( IS_L_TYPE(insn_word) ) {
		insn = &fusion_insn_L[ GET_FUNCT_L(insn_word) ];
		fpr(stream, "%s\t%s,\t0x%03x(%s)", insn->name, \
						fusion_gpreg_name[ GET_RD(insn_word) ],\
						GET_IMM_L(insn_word),\
						fusion_gpreg_name[ GET_RSA(insn_word) ] );
	
	} else if( IS_LI_TYPE(insn_word) ) {
		insn = &fusion_insn_LI[ GET_DSEL_LI(insn_word) ];
		fpr(stream, "%s\t%s,\t0x%04lx", insn->name, \
						fusion_gpreg_name[ GET_RD(insn_word) ],(unsigned long int) GET_IMM_LI(insn_word) );
	
	} else if( IS_S_TYPE(insn_word) ) {
		insn = &fusion_insn_S[ GET_FUNCT_S(insn_word) ];
		fpr(stream, "%s\t%s,\t0x%03x(%s)", insn->name, \
						fusion_gpreg_name[ GET_RSB(insn_word) ],\
						GET_IMM_S(insn_word),\
						fusion_gpreg_name[ GET_RSA(insn_word) ] );
	
	} else if((opc == OPC_JMP) || (opc == OPC_JLNK)){// else if( IS_J_TYPE(insn_word) ) {
		if( GET_RSA(insn_word) == 0x00 ) { //if just a normal jump
			if ( IS_JLNK_INSN( insn_word ) ) //if jump and link
				fpr(stream, "jal\t%d", (signed ) SEXT_21B(GET_IMM_J(insn_word))  );
			else if (IS_JMP_INSN( insn_word ) )
				fpr(stream, "j\t%d", (signed) SEXT_21B(GET_IMM_J(insn_word))  );
			else
				fpr(stream, "nri"); //not a real jump
		} else { //using register, no possible error here
			if ( IS_JLNK_INSN( insn_word ) ) //jump register and link
				fpr(stream, "jrl\t0x%06lx(%s)", (unsigned long int) GET_IMM_J(insn_word), \
								fusion_gpreg_name[ GET_RSA(insn_word) ] );
			else if (IS_JMP_INSN( insn_word ) ) //jump register
				fpr(stream, "jr\t0x%06lx(%s)", (unsigned long int) GET_IMM_J(insn_word), \
								fusion_gpreg_name[ GET_RSA(insn_word) ] );
			else
				fpr(stream, "nri"); //not a real jump
		}
	
	} else if( IS_B_TYPE(insn_word) ) {
			insn = &fusion_insn_B[ GET_FUNCT_B(insn_word) ];
			fpr(stream, "%s\t%s,\t%s,\t%d", insn->name,\
							fusion_gpreg_name[ GET_RSA(insn_word) ],\
							fusion_gpreg_name[ GET_RSB(insn_word) ],\
							(signed) SEXT_14B(GET_IMM_B(insn_word)) );
	
	} else if( IS_SYS_TYPE(insn_word) ) {
			insn = &fusion_insn_SYS[ GET_FUNCT_SYS(insn_word) ];
		switch(insn->args) { //determine what operands, uses immediate	
			case USE_NONE: //no operands
				fpr(stream, "%s", insn->name);
				break;
			case USE_I: //only immediate
				fpr(stream, "%s\t0x%02x", insn->name, GET_IMM_SYS(insn_word));
				break;
			case USE_RAI: //RSa and immediate
				fpr(stream, "%s\t%s,\t0x%02x", insn->name,\
								fusion_gpreg_name[ GET_RSA(insn_word) ],\
								GET_IMM_SYS(insn_word));
				break;
			case USE_RDA:

				//check if instruction is accessing system registers
				if( GET_FUNCT_SYS(insn_word) == 0x02 )	{
					fpr(stream, "%s\t%s,\t%s", insn->name,\
							fusion_spreg_name[ GET_RD(insn_word) ],\
							fusion_gpreg_name[ GET_RSA(insn_word)] );
					break;
				} else if(GET_FUNCT_SYS(insn_word) == 0x03 ) {
					fpr(stream, "%s\t%s,\t%s", insn->name,\
							fusion_gpreg_name[ GET_RD(insn_word) ],\
							fusion_spreg_name[ GET_RSA(insn_word)] );
					break;
				} else {
				
					break;
				}
			case USE_RDI: //Rd and immediate
					fpr(stream, "%s\t%s,\t0x%02x", insn->name,\
								fusion_gpreg_name[ GET_RD(insn_word) ],\
								 GET_IMM_SYS(insn_word)  );
				break;			
			case USE_RDAI: //Rd, RSa, and immediate
					fpr(stream, "%s\t%s,\t%s,\t0x%02x", insn->name,\
								fusion_gpreg_name[ GET_RD(insn_word) ],\
								fusion_gpreg_name[ GET_RSA(insn_word) ],\
								GET_IMM_SYS(insn_word) );
					break;
			default:
				fpr(stream, "nri_sys"); //not a real instruction
				break;
		}
		
			
	
	} else if( IS_CP_INSN(insn_word) ) {
		fpr( stream, "cpinsn?"); //coprocessor instruction, should fix
								//this soon
	} else if( insn_word == 0x00000000){
		fpr(stream, "nop"); //only for nops
	}else {
		fpr( stream, "nri_unk"); //not real instruction, or just unknown
	}


	return 4;

	fail:
		info->memory_error_func (status, addr, info);
		return -1;
}
