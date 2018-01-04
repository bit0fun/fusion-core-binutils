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
//#include "opcode/fusion-opc.h"
#include "elf/fusion.h"
#include <stdint.h>


#ifndef ECOFF_DEBUGGING
#define NO_ECOFF_DEBUGGING
#define ECOFF_DEBUGGING 0
#endif



//extern const fusion_inst_t fusion_inst[128];
extern const fusion_opc_info_t fusion_insn_R[NUM_INSN_R];
extern const fusion_opc_info_t fusion_insn_I[NUM_INSN_I];
extern const fusion_opc_info_t fusion_insn_L[NUM_INSN_L];
extern const fusion_opc_info_t fusion_insn_LI[NUM_INSN_LI];
extern const fusion_opc_info_t fusion_insn_S[NUM_INSN_S];
extern const fusion_opc_info_t fusion_insn_J[NUM_INSN_J];
extern const fusion_opc_info_t fusion_insn_JL[NUM_INSN_JL];
extern const fusion_opc_info_t fusion_insn_B[NUM_INSN_B];
extern const fusion_opc_info_t fusion_insn_SYS[NUM_INSN_SYS];
extern const fusion_opc_info_t fusion_insn_all[NUM_INSN];



const char comment_chars[]			="#";
const char line_separator_chars[] 	=";";
const char line_comment_chars[]		="#";

static int pending_reloc;
static struct hash_control *opcode_hash_control;

const pseudo_typeS md_pseudo_table[] = {
	
	{"byte", s_cons, 0},}
	{"half", s_cons, 2},
	{"word", s_cons, 4},
	{ NULL, NULL, 0 },

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

static unsigned elf_flags = 0;
/*options that .option pseudo-op can change*/
struct fusion_set_options{
	int relax; /* tell the linker which relocations it can relax (optimize)  */

};

static struct fusion_set_options fusion_options = {
	1,	//relax
};


/*How to append instruction to output; using MIPS file as reference*/
enum append_method {
	APPEND_ADD,	 							//normal append
	APPEND_ADD_WITH_NOP,					//add, then put NOP after
	APPEND_SWAP								//swap instruction with last one
	/*no delay slot usage, fuck delay slots*/

};
/* Handle of the OPCODE hash table*/
static struct hash_control *op_hash = NULL;

/* Macros for relaxation for branches and far jumps*/


/*instruction information, format, operands, fix requirements*/
struct fusion_cl_insn {
	/* opcode entry in fusion_opc_info_t*/
	const struct fusion_opc_info_t *insn_mo;

	/*actual instruction bits*/
	insn_t insn_word;

	/* code fragment that the instruction is in*/
	struct frag *frag;

	/* offset inside of fragment where the first byte of the instruction is*/
	long frag_offset;

	/* Relocations associated with instruction*/
	fixS *fixptr;
	

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

/* initialize instruction from opcode entry. don't define position yet*/
static void create_insn(struct fusion_cl_insn *insn, const struct fusion_opc_info_t *mo){
	insn->insn_mo = mo;
	insn->insn_word = mo->opc;
	insn->frag = NULL;
	insn->frag_offset = 0;
	insn->fixptr = NULL;
}

/* Puts instruction at the location specified by frag, and frag_offset*/
static void insert_insn(const struct fusion_cl_insn *insn){
	char *f = insn->frag->fr_literal + insn->frag_offset;
	md_number_to_chars(f, insn->insn_opc, 4); //4 since 32 bit instruction

}

/*move instruction to the offset in the fragment. adjust fixes as necessary
 * and put the opcode in new location*/
static void move_insn(struct fusion_cl_insn *insn, fragS* frag, long frag_offset){
	insn->frag = frag;
	insn->frag_offset = frag_offset;
	if(insn->fixptr != NULL){
		insn->fixptr->fx_frag = frag;
		insn->fixptr->fx_where = frag_offset;
	}
	insert_insn(insn);
}

/*add insn to end of output*/
static void add_fixed_insn(struct fusion_cl_insn* insn){
	char* f = frag_more(4); //4 since 32 bit instructions only
	move_insn(insn, frag_now, f - frag_now->fr_literal);
}

static void add_relaxed_insn(strct fusion_cl_insn* insn, int max_chars, int var,
		relax_substateT subtype, symbolS* symbol, offsetT offset){
	frag_grow(max_chars);
	move_insn(insn, frag_now, frag_more(0) - frag_now->fr_literal);
	frag_var(rs_machine_dependent, max_chars, var,
		       	subtype, symbol, offset, NULL);

}

//not using for now, add in relaxation after getting main compiler working
/* Figure out length of branch sequence, adjust stored length accordingly.
 * If FRAGP is NULL, worst case length is returned.*/
/*
static unsigned relaxed_branch_length(fragS* fragp, asection* section, int update){
	int jump

}
*/


struct regname{
	const char* name;
	unsigned int	num;
};

enum reg_file{
	REGF_GPR,
	REGF_SYS,
	REGF_MAX
};

static struct hash_constrol* reg_names_hash = NULL;

#define ENCODE_REG_HASH(rf, n)\
	((void*)(uintptr_t)((n) * REGF_MAX + (rf) + 1))
#define DECODE_REG_FILE(hash) (((uintptr_t)(hash) - 1) % REGF_MAX)
#define DECODE_REG_NUM(hash) (((uintptr_t)(hash) - 1) / REGF_MAX)
static void hash_reg_name(enum reg_file regf, const char* name, unsigned n){
	void* hash = ENCODE_REG_HASH(regf, n);
	const char* retval = hash_insert(reg_names_hash, name, hash); 

	if(retval !=NULL){
		as_fatal(_("internal error: can't hash `%s': %s"), name, hash);
	}

}
static void hash_reg_names(enum reg_file regf, const char* const names[], unsigned n){
	unsigned i;
	for(i = 0; i < n; i++){
		hash_reg_name(class, names[i], i);
	}

}

static unsigned int reg_lookup_internal(const char* s, enum reg_file regf){
	struct regname* r = (struct regname* )hash_find(reg_names_hash, s);
	if( r == NULL || DECODE_REG_FILE(r) != regf)
		return -1;
	return DECODE_REG_NUM(r);
}


void md_operand (expressionS *op __attribute__((unused))){

	/*whyyyy does this matter*/
}

void md_begin (void) {
	int i;
	op_hash_ctrl = hash_new();

	while(fusion_insn_all[i].name){
		const char* name = fusion_insn_all[i].name;
		const char* hash_error = 
			hash_insert(op_hash_ctrl, name, (void*) &fusion_insn_all[i]);
		if(hash_error){
			fprintf(stderr, _("internal error: can't hash `%s': %s\n"),
					fusion_insn_all[i].name, hash_error);
			as_fatal(_("Assembler is broken. I'm not touching that file, fix me plz."));
		}
				
	}
	reg_names_hash = hash_new();
	hash_reg_names(REGF_GPR, fusion_gpreg_name, NGPR);
	hash_reg_names(REGF_GPR, fusion_gpreg_num, NGPR);

#define DECLARE_SYSREG(name, num)	hash_reg_name(REGF_SYS, #name, num);
#include "opcode/fusion-opc.h"
/*
	for (count = 0, opcode = fusion_insn_R; count++< NUM_INSN_R; opcode++){
		hash_error = hash_insert(opcode_hash_control, opcode->name, (char *) opcode);
		if(hash_error){
			fprintf(stderr, _("internal error: can't hash `%s': %s\n"),
					fusion_insn_R);
		}
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
*/
	//target_big_endian = TARGET_BYTES_BIG_ENDIAN;
	record_alignment(text_section, 2);
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

	 if(*s != '$') { //denote register with $ 
		as_bad(_("expecting register"))
		ignore_rest_of_line();
		return -1;
	 }
	
	//if zero register?
	 if(    (s[1] == 'z')
	     && (s[2] == 'e')
	     && (s[3] == 'r')
		 && (s[4] == 'o') ){
		*ptr += 4; 
		return 0; //register 0
	 } else if( (s[1] == 's') && (s[2] == 'p')) {
			*ptr += 2;
			return 1;
	 } else if( (s[1] == 'f') && (s[2] == 'p')){
	 		*ptr += 2;
			return 2;
	 } else if( (s[1] == 'g') && (s[2] == 'p')) {
			*ptr += 2;
			return 3;
	 } else if( (s[1] == 'r') && (s[2] == 'a')) {
			*ptr += 2;
			return 4;
	 } else if( (s[1] == 'a') && (s[2] == 'r') && (s[3] == 'g')) {
			reg = s[4] - '0'; //get number value
			if( (reg <0) || (reg > 3) ){ //not ARGX reg
				as_bad(_("illegal argument register"));
				ignore_rest_of_line();
				return -1;
			} else {
				*ptr +=5;	
				return reg + 5; //registers 5 through 8
			}
			
	 } else if( (s[1] == 'r') 
			  &&(s[2] == 'v')
			  &&(s[3] == 'a')
			  &&(s[4] == 'l')){
	 	reg = s[5] - '0';
		if( (reg < 0) || (reg >1) ){
			as_bad(_("illegal return value regsiter"));
			ignore_rest_of_line();
			return -1;
		} else{
			*ptr += 6;
			return reg + 9; //regs 9 and 10
		}
	 
	 } else if ( (s[1] == 'g') && (s[2] == 'r')){
		reg = s[3] - '0'; 
		if( (reg < 0) || (reg > 9) ){
			as_bad(_("illegal general use register"));	
			ignore_rest_of_line();
			return -1;
		} else{
			if( reg == 1) { //need to check if 10
				reg = (s[4] - '0') + 10; 	
				if(reg != 10){ //only option at the moment
					as_bad(_("illegal general use register"));	
					ignore_rest_of_line();
					return -1;
				} else {
					*ptr += 5;
					return 21; //gp10 is R21
				}
			} else {
				*ptr += 4;
				return reg + 11; //gp0 is R11
			}
		}
	 
	 
	 } else if ( (s[1] == 't') && (s[2] == 'm') 
				&& (s[3] == 'p')  ){
			reg = s[4] - '0'; 
		if( (reg < 0) || (reg > 8) ){
			as_bad(_("illegal general use register"));	
			ignore_rest_of_line();
			return -1;
		} else {
			*ptr += 5;
			return reg + 22; //tmp0 is R22
		}
	 
	 } else if( (s[1] == 'h') && (s[2] == 'i') && (s[3] == '0')){
	 	*ptr += 3;
		return 30; //HI0 is R30
	 } else if( (s[1] == 'l') && (s[2] == 'o') && (s[3] == 'w')
					 && (s[4] == '0')){
	 	*ptr += 5;
		return 31; //LOW0 is R31
	 } else {
			as_bad(_("expecting register, unknown operand"));	
			ignore_rest_of_line();
			return -1; 
	 }

}

/*Write out instruction*/
static void append_insn(struct fusion_cl_insn* insn_info, expressionS* addr_expr, 
		bfd_reloc_code_real_type reloc_type){

	dwarf2_emit_insn(0);
	if(reloc_type != BFD_RELOC_UNUSED){
		reloc_howot_type *howto;
		gas_assert(addr_expr);

		if( (reloc_type == BFD_RELOC_FUSION_14_PCREL) 
		  || (reloc_type == BFD_RELOC_FUSION_21_PCREL) ){
			int j = reloc_type == BFD_RELOC_FUSION_21_PCREL;
			int best_case = 2;
			unsigned worst_case = relaxed_branch_length(NULL, NULL, 0);
			add_relaxed_insn (insn_info, worst_case, best_case,
					RELAX_BRANCH_ENCODE(j, worst_case),
					addr_expr->X_add_symbol,
					addr_expr->X_add_number);
			return;
		  }
		else{
			howto = bfd_reloc_type_lookup(stdoutput, reloc_type);
			if(howto == NULL){
				as_bad(_("Unsupported Fusion relocation number: %d"), reloc_type);
			}
			insn_info->fixptr = fix_new_exp(insn_info->frag, insn_info->frag_offset,
					bfd_get_reloc_size(howto), addr_expr, FALSE, reloc_type);
			insn_info->fixptr=fx_tcbit = fusion_set_opts.relax;
		}
	
	}

	add_fixed_insn(insn_info);
	insert_insn(insn_info);
	


}


void md_assemble(char *str){
	char *op_start;
	char *op_end;

	fusion_opc_info_t *opcode;
	char *output;
	int index = 0;
	char pend;
	int nlen = 0;

	struct fusion_cl_insn insn;
	expressionS imm_expr;
	bfd_reloc_code_real_type imm_reloc = BFD_RELOC_UNUSED;

	const char* error = fusion_ip(str, &insn, &imm_expr, &imm_reloc);
	if(error){
		as_bad("%s `%s'", error, str);
		return;
	}
	/* checking for relative memory acess*/
//	insn_t opc_check = insn.insn_mo->opc;
//	if(opc_check == ){
	
//	}
//	else{
		append_insn(&insn, &imm_expr, imm_reloc);
//	}	

//
//	/*drop leading whitespace*/
//	while(*str == ' '){
//		str++;
//	}
//
//	/*find opcode end*/
//	op_start = str;
//	for(op_end = str; *op_end && !is_end_of_line[*op_end & 0xff] && *op_end != ' '; op_end++)
//		nlen++;
//
//	pend = *op_end;
//	if(nlen == 0)
//		as_bad (_("can't find opcode "));
//
//	opcode = (fusion_opc_info_t *) hash_find(opcode_hash_control, op_start);
//	*op_end = pend;
//
//	if(opcode == NULL){
//		as_bad(_("unknown opcode %s"),op_start);
//		return;
//	}
//
//	output = frag_more(1);
//	output[index++] = opcode->opc;
//
//	while(ISSPACE(*op_end)){
//		op_end++;
//	}
//	
//	if(*op_end != 0){
//		as_warn("extra stuff on line ignored");
//	}
//
//	if(pending_reloc){
//		as_bad("something forgot to clean up\n");
//	}
//	*/

}

/*turn a string in input_line_pointer to fp constant, store in
 * *LITP. error return, null on ok*/

/*Assembles instruction to binary format*/
static const char* fusion_ip(char *str, struct fusion_cl_insn *ip, expressionS* imm_expr,
		bfd_reloc_code_real_type* imm_reloc){

	char* s;
	const char* args;
	struct fusion_opc_info_t* insn;
	char *argsStart;
	unsigned int regnum;
	char save_c = 0;
	int argnum;
	const struct percent_op_match* p;
	const char* error = "unrecognized opcode";

	for(s = str; *s != '\0'; ++s){
		if(ISSPACE(*s)){
			save_c = *s;
			*s++ = '\0';
			break;
		}
	}
	insn = (struct fusion_opc_info_t *) hash_find(op_hash, str);

	argsStart = s;




}

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

