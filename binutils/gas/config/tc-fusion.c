/* tc-fusion.c -- Assemble code for Fusion-Core
   Copyright (C) 2009-2017 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* Contributed by Dylan Wadler  */

/*Clean up defines*/
#undef OBJ_PROCESS_STAB
#undef OUTPUT_FLAVOR
#undef S_GET_ALIGN
#undef S_GET_SIZE
#undef S_SET_ALIGN
#undef S_SET_SIZE
#undef obj_frob_file
#undef obj_frob_file_after_relocs
#undef obj_frob_symbol
#undef obj_pop_insert
#undef obj_sec_sym_ok_for_reloc
#undef OBJ_COPY_SYMBOL_ATTRIBUTES



#include "as.h"
#include "config.h"
#include "safe-ctype.h"
#include "opcode/fusion.h"
#include "elf/fusion.h"
#include <stdint.h>


#ifndef ECOFF_DEBUGGING
#define NO_ECOFF_DEBUGGING
#define ECOFF_DEBUGGING 0
#endif



//extern const fusion_inst_t fusion_inst[128];
extern const fusion_insn_t fusion_insn_R[NUM_INSN_R];
extern const fusion_insn_t fusion_insn_I[NUM_INSN_I];
extern const fusion_insn_t fusion_insn_L[NUM_INSN_L];
extern const fusion_insn_t fusion_insn_LI[NUM_INSN_LI];
extern const fusion_insn_t fusion_insn_S[NUM_INSN_S];
extern const fusion_insn_t fusion_insn_J[NUM_INSN_J];
extern const fusion_insn_t fusion_insn_JL[NUM_INSN_JL];
extern const fusion_insn_t fusion_insn_B[NUM_INSN_B];
extern const fusion_insn_t fusion_insn_SYS[NUM_INSN_SYS];



const char comment_chars[]			="#";
const char line_separator_chars[] 	=";";
const char line_comment_chars[]		="#";

static int pending_reloc;
static struct hash_control *opcode_hash_control;

const pseudo_typeS md_pseudo_table[] = {
	
	{ 0, 0, 0 }

};

const char FLT_CHARS[] = "rRsSfFdDxXpP";
const char EXP_CHARS[] = "eE";


#ifdef DEBUG
#define DBG(x) printf x
#else
#define DBG(x)
#endif

#define streq(a, b)		(strcmp (a, b) == 0)

#define SKIP_SPACE_TABS(S) \
	do { while (*(S) == ' ' || *(S) == '\t') ++(S); } while(0)


#define RDATA_SECTION_NAME ".rodata"
extern int target_big_endian;



/*How to append instruction to output; using MIPS file as reference*/
enum append_method {
	APPEND_ADD,	 							//normal append
	APPEND_ADD_WITH_NOP,					//add, then put NOP after
	APPEND_SWAP								//swap instruction with last one
	/*no delay slot usage, fuck delay slots*/

};

/*instructio information, format, operands, fix requirements*/
struct fusion_cl_insn {
	/*struct for */
	const struct fusion_opcode *insn_op;
	unsigned long insn_word;


};

static int auto_align = 1; //not sure if this is used

/* Below comments taken from tc-mips.c*/
/* To output NOP instructions correctly, we need to keep information
   about the previous two instructions.  */

/* Whether we are optimizing.  The default value of 2 means to remove
   unneeded NOPs and swap branch instructions when possible.  A value
   of 1 means to not swap branches.  A value of 0 means to always
   insert NOPs.  */
static int fusion_optimize = 2;

/* Below comment taken from tc-mips.c*/
/* Debugging level.  -g sets this to 2.  -gN sets this to N.  -g0 is
   equivalent to seeing no -g option at all.  */
static int fusion_debug = 0;


/* NOP number requirements*/
#define MAX_NOPS		4  //maximum NOPs for any purpose
#define MAX_DELAY_NOPS	2  //maximum NOPs for delays

/*list of previous instructions, index 0 most recent*/
static struct fusion_cl_insn history[1 + MAX_NOPS];

void md_operand (expressionS *op __attribute__((unused))){

	/*whyyyy does this matter*/
}

void md_begin (void) {
	int count;
	const fusion_insn_t *opcode;
	opcode_hash_control = hash_new();

	for (count = 0, opcode = fusion_insn_R; count++< NUM_INSN_R; opcode++){
		hash_insert(opcode_hash_control, opcode->name, (char *) opcode);
	} 
	for (count = 0, opcode = fusion_insn_I; count++< NUM_INSN_I; opcode++){
		hash_insert(opcode_hash_control, opcode->name, (char *) opcode);
	} 
	for (count = 0, opcode = fusion_insn_L; count++< NUM_INSN_L; opcode++){
		hash_insert(opcode_hash_control, opcode->name, (char *) opcode);
	} 
	for (count = 0, opcode = fusion_insn_LI; count++< NUM_INSN_LI; opcode++){
		hash_insert(opcode_hash_control, opcode->name, (char *) opcode);
	} 
	for (count = 0, opcode = fusion_insn_S; count++< NUM_INSN_S; opcode++){
		hash_insert(opcode_hash_control, opcode->name, (char *) opcode);
	} 
	for (count = 0, opcode = fusion_insn_J; count++< NUM_INSN_J; opcode++){
		hash_insert(opcode_hash_control, opcode->name, (char *) opcode);
	} 
	for (count = 0, opcode = fusion_insn_B; count++< NUM_INSN_B; opcode++){
		hash_insert(opcode_hash_control, opcode->name, (char *) opcode);
	} 
	for (count = 0, opcode = fusion_insn_SYS; count++< NUM_INSN_SYS; opcode++){
		hash_insert(opcode_hash_control, opcode->name, (char *) opcode);
	} 

	//target_big_endian = TARGET_BYTES_BIG_ENDIAN;
	bfd_set_arch_mach(stdoutput, bfd_arch_fusion, 0);
}

/*Parse functions*/

/*Parse expression, then restore input line pointer*/
static char* parse_expt_save_ilp(char *s, expressionS* op){
	char* save = input_line_pointer;
	input_line_pointer = s;
	expression(op);
	s = input_line_pointer;
	input_line_pointer = save;
	return s;
}

/*Parse register for operands*/
static int parse_register_operand(char** ptr){
	 int reg;
	 char* s = *ptr;

	 //if(*s != '$') { //denote register with $ 
	//	as_bad(_("expecting register"))
	//	ignore_rest_of_line();
	//	return -1;
	// }
	
	//if zero register?
	 if(    (s[0] == 'z')
	     && (s[1] == 'e')
	     && (s[2] == 'r')
		 && (s[3] == 'o') ){
		*ptr += 3; 
		return 0; //register 0
	 } else if( (s[0] == 's') && (s[1] == 'p')) {
			*ptr += 2;
			return 1;
	 } else if( (s[0] == 'f') && (s[1] == 'p')){
	 		*ptr += 2;
			return 2;
	 } else if( (s[0] == 'g') && (s[1] == 'p')) {
			*ptr += 2;
			return 3;
	 } else if( (s[0] == 'r') && (s[1] == 'a')) {
			*ptr += 2;
			return 4;
	 } else if( (s[0] == 'a') && (s[1] == 'r') && (s[2] == 'g')) {
			reg = s[3] - '0'; //get number value
			if( (reg <0) || (reg > 3) ){ //not ARGX reg
				as_bad(_("illegal argument register"));
				ignore_rest_of_line();
				return -1;
			} else {
				*ptr +=4;	
				return reg + 5; //registers 5 through 8
			}
			
	 } else if( (s[0] == 'r') 
			  &&(s[1] == 'v')
			  &&(s[2] == 'a')
			  &&(s[3] == 'l')){
	 	reg = s[4] - '0';
		if( (reg < 0) || (reg >1) ){
			as_bad(_("illegal return value regsiter"));
			ignore_rest_of_line();
			return -1;
		} else{
			*ptr += 5;
			return reg + 9; //regs 9 and 10
		}
	 
	 } else if ( (s[0] == 'g') && (s[1] == 'r')){
		reg = s[2] - '0'; 
		if( (reg < 0) || (reg > 9) ){
			as_bad(_("illegal general use register"));	
			ignore_rest_of_line();
			return -1;
		} else{
			if( reg == 1) { //need to check if 10
				reg = (s[3] - '0') + 10; 	
				if(reg != 10){ //only option at the moment
					as_bad(_("illegal general use register"));	
					ignore_rest_of_line();
					return -1;
				} else {
					*ptr += 4;
					return 21; //gp10 is R21
				}
			} else {
				*ptr += 3;
				return reg + 11; //gp0 is R11
			}
		}
	 
	 
	 } else if ( (s[0] == 't') && (s[1] == 'm') 
				&& (s[2] == 'p')  ){
			reg = s[3] - '0'; 
		if( (reg < 0) || (reg > 8) ){
			as_bad(_("illegal general use register"));	
			ignore_rest_of_line();
			return -1;
		} else {
			*ptr += 4;
			return reg + 22; //tmp0 is R22
		}
	 
	 } else if( (s[0] == 'h') && (s[1] == 'i') && (s[2] == '0')){
	 	*ptr += 3;
		return 30; //HI0 is R30
	 } else if( (s[0] == 'l') && (s[1] == 'o') && (s[2] == 'w')
					 && (s[3] == '0')){
	 	*ptr += 4;
		return 31; //LOW0 is R31
	 } else {
			as_bad(_("expecting register, unknown operand"));	
			ignore_rest_of_line();
			return -1; 
	 }

}

void md_assemble(char *str){
	char *op_start;
	char *op_end;

	fusion_insn_t *opcode;
	char *output;
	int index = 0;
	char pend;
	int nlen = 0;

	/*drop leading whitespace*/
	while(*str == ' '){
		str++;
	}

	/*find opcode end*/
	op_start = str;
	for(op_end = str; *op_end && !is_end_of_line[*op_end & 0xff] && *op_end != ' '; op_end++)
		nlen++;

	pend = *op_end;
	if(nlen == 0)
		as_bad (_("can't find opcode "));

	opcode = (fusion_insn_t *) hash_find(opcode_hash_control, op_start);
	*op_end = pend;

	if(opcode == NULL){
		as_bad(_("unknown opcode %s"),op_start);
		return;
	}

	output = frag_more(1);
	output[index++] = opcode->opcode;

	while(ISSPACE(*op_end)){
		op_end++;
	}
	
	if(*op_end != 0){
		as_warn("extra stuff on line ignored");
	}

	if(pending_reloc){
		as_bad("something forgot to clean up\n");
	}

}

/*turn a string in input_line_pointer to fp constant, store in
 * *LITP. error return, null on ok*/

const char* md_atof(int type, char* litP, int* sizeP){
	return ieee_md_atof(type, litP, sizeP, target_big_endian);
}

void md_number_to_chars(char *buf, valueT val, int n){
//	if(target_big_endian)
		number_to_chars_bigendian(buf, val, n);
//	else
//		number_to_cahrs_littleendian(buf, val, n);

}

const char *md_shortopts ="";

struct option md_longopts[] = {
	{NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof(md_longopts);


/*target specific options*/
int md_parse_option(int c ATTRIBUTE_UNUSED, const char *arg ATTRIBUTE_UNUSED){
	return 0;
}

void md_show_usage(FILE* stream ATTRIBUTE_UNUSED){
}

void md_apply_fix(fixS *fixP ATTRIBUTE_UNUSED, valueT* valP ATTRIBUTE_UNUSED, segT seg ATTRIBUTE_UNUSED){
}

arelent *tc_gen_reloc(asection* section ATTRIBUTE_UNUSED, fixS *fixp){
	arelent* rel;
	bfd_reloc_code_real_type r_type;

	rel = xmalloc( sizeof(arelent) );
	rel->sym_ptr_ptr = xmalloc( sizeof( asymbol* ) );
	*rel->sym_ptr_ptr= symbol_get_bfdsym(fixp->fx_addsy);
	rel->address = fixp->fx_frag->fr_address + fixp->fx_where;

	r_type = fixp->fx_r_type;
	rel->addend = fixp->fx_addnumber;
	rel->howto = bfd_reloc_type_lookup(stdoutput, r_type);

	if(rel->howto == NULL){
		as_bad_where(fixp->fx_file, fixp->fx_line, _("Cannot represent relocation type %s"),bfd_get_reloc_code_name(r_type));
		/*set to garbage value to continue usage*/
		rel->howto = bfd_reloc_type_lookup(stdoutput, BFD_RELOC_32);
		gas_assert(rel->howto != NULL);
	
	}

	return rel;
}

