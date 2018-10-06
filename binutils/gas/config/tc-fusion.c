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

/* Contributed by Dylan Wadler: dylan@fusion-core.org  */

#define WORST_CASE	4
//#define DEBUG
#define HAVE_64BIT_ADDRESSES 0 //no 64 bit addressing yet

#include "as.h"
#include "config.h"
#include "ctype.h"
#include "opcode/fusion.h"
#include "opcode/fusion-opc.h"
#include "elf/fusion.h"

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

	/*True if entry can't be moved from current position*/
	unsigned int fixed_p : 1;
	
};


/*prototypes*/
void fusion_pop_insert(void);

int parse_rdab(int* rd, int* rsa, int* rsb, char* op_end);
int parse_rda( int* rd, int* rsa, char* op_end);
int parse_rab( int* rsa, int* rsb, char* op_end);
int parse_imm( int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip);

int parse_rdi( int* rd, int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip);
int parse_rdai( int* rd, int* rsa, int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip);
int parse_rai(int *rsa, int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip);
int parse_rabi(int* rsa, int* rsb, int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip);

int parse_rdai_offset( int* rd, int* rsa, int* imm,  char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip);
int parse_rai_offset(int *rsa, int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip);
int parse_rabi_offset(int* rsa, int* rsb, int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip);

bfd_reloc_code_real_type return_reloc(fusion_opc_info_t* insn, char* str);
static int parse_register_operand(char** ptr);

//bfd_boolean assemble_insn_bin(char* str, fusion_opc_info_t* insn, insn_t* insn_bin);
bfd_boolean assemble_insn_bin(char* str, struct fusion_cl_insn* insn, expressionS* imm_expr, bfd_reloc_code_real_type* imm_reloc);
static void append_insn(struct fusion_cl_insn* ip, expressionS* address_expr,
		bfd_reloc_code_real_type reloc_type);



extern const fusion_opc_info_t fusion_insn_all[NUM_INSN];

extern const char* fusion_gpreg_name[32];
extern const char* fusion_gpreg_num[32];
extern const char* fusion_spreg_name[13];


const char comment_chars[]			="#";
const char line_separator_chars[] 	=";";
const char line_comment_chars[]		="#";

static char *expr_end;

static int pending_reloc;

struct fusion_set_options{
	int pic;
	int relax;
};

//static struct fusion_set_options fusion_opts = {
//	0,	/* pic   */
//	1,	/* relax */
//};

static struct hash_control *op_hash_ctrl;

const pseudo_typeS md_pseudo_table[] = {
	
//	{"byte", s_cons, 0},}
//	{"half", s_cons, 2},
//	{"word", s_cons, 4},
//	{"gpword", s_gpword, 0 },
//	{"gpvalue", s_gpvalue, 0},
//	{"bss", s_change_sec, 'b'},
//	{"align", s_align, 0},
//	{"global", s_fusion_globl, 0},
//	{"globl", s_fusion_globl, 0},
//	{"text", s_change_sec, 	't'},
	{ NULL, NULL, 0 },


};

const char FLT_CHARS[] = "rRsSfFdDxXpP";
const char EXP_CHARS[] = "eE";

#define INSERT_OPERAND(OPERAND_GEN, INSN)\
		((INSN).insn_word |= OPERAND_GEN) 

#define RELAX_BRANCH_ENCODE(uncond, length)\
		((relax_substateT) \
		(0xc00000000 \
		 | ((uncond) ? 1 : 0) \
		 | ((length) << 2)))

/* Is value sign extended 32 bit value*/
#define IS_SEXT_32BIT_NUM(x)\
	(((x) &~ (offsetT) 0x7fffffff) == 0 \
	 || (((x) &~ (offsetT) 0x7fffffff) == ~(offsetT) 0x7fffffff))

/* Is the given value a sign-extended 16-bit value?  */
#define IS_SEXT_16BIT_NUM(x)						\
  (((x) &~ (offsetT) 0x7fff) == 0					\
   || (((x) &~ (offsetT) 0x7fff) == ~ (offsetT) 0x7fff))

/* Is value zero extended 32 bit value, or negated*/
#define IS_ZEXT_32BIT_NUM(x)\
	(((x) &~ (offsetT) 0xffffffff) == 0 \
	 || (((x) &~ (offsetT) 0x7fffffff) == ~(offsetT) 0xffffffff))


//static valueT md_chars_to_number(char* buf, int n);
extern int target_big_endian;

void md_operand(expressionS* op __attribute__((unused))){
	/*empty for now*/

}

#define streq(a, b)		(strcmp (a, b) == 0)

#define SKIP_SPACE_TABS(S) \
	do { while (*(S) == ' ' || *(S) == '\t') ++(S); } while(0)
#define is_immediate_prefix(C)	((C) == '%')	

#define RDATA_SECTION_NAME ".rodata"



/* expression in macro instruction; set by fusion_ip 
 * when populated, it is always O_constant */

//static expressionS imm_expr;

/* returns instrution length */

//static inline unsigned int insn_length( const struct fusion_cl_insn* insn ){
	/* this may change for support for various new co-processors */
//	return 4;
//}

extern int target_big_endian;


/*list of previous instructions, index 0 most recent*/
//static struct fusion_cl_insn history[1 + MAX_NOPS];

/* initialize instruction from opcode entry. don't define position yet*/

static void create_insn(struct fusion_cl_insn *insn, const struct fusion_opc_info_t *mo){
	insn->insn_mo = mo;
	insn->insn_word = mo->opc;
	insn->frag = NULL;
	insn->frag_offset = 0;
	insn->fixptr = NULL;
};

/* Puts instruction at the location specified by frag, and frag_offset*/

static void install_insn(const struct fusion_cl_insn *insn){
	char *f = insn->frag->fr_literal + insn->frag_offset;
	md_number_to_chars(f, insn->insn_word, 4);//insn_length(insn)); //4 since 32 bit instruction

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
	install_insn(insn);
}


/*add insn to end of output*/

static void add_fixed_insn(struct fusion_cl_insn* insn){
	char* f = frag_more(4); //4 since 32 bit instructions only
	move_insn(insn, frag_now, f - frag_now->fr_literal);
}

/* start a variant fragment and move instruction to the start of the variant
 * part marking it as fixed*/
/*
static void add_relaxed_insn(struct fusion_cl_insn* insn, int max_chars,
		int var, relax_substateT subtype, symbolS* symbol, 
		offsetT offset){
	frag_grow(max_chars);
		move_insn(insn, frag_now, frag_more(0) - frag_now->fr_literal);
		insn->fixed_p = 1;
		frag_var(rs_machine_dependent, max_chars, var, subtype, symbol, offset,
				NULL);

}
*/
/*compute length of branch sequence, adjust stored length accordingly
 * if fragp is null, worst case length is returned*/
/*
static unsigned relaxed_branch_length(fragS *fragp, asection* sec, int update){
		int jump, length = 8;
		if(!fragp)
			return length;
		//jump = RELAX_BRANCH_UNCOND(fragp->fr_subtype);
		//length = RELAX_BRANCH_LENGTH(fragp->fr_subtype);

	//	length = jump ? 4 : 8;
		if(fragp->fr_symbol != NULL
				&& S_IS_DEFINED(fragp->fr_symbol)
				&& !S_IS_WEAK(fragp->fr_symbol)
				&& sec == S_GET_SEGMENT(fragp->fr_symbol)){

			offsetT val = S_GET_VALUE(fragp->fr_symbol) + fragp->fr_offset;
			val -= fragp->fr_address + fragp->fr_fix;
		}
	if(update){
		fragp->fr_subtype = RELAX_BRANCH_ENCODE(jump, length);
	}

	return length;

}
*/
//not using at the moment

/*insert N copies of INSN into history buffer, starting at first.
 * netierh first nor n need to be clipped*/
//static void insert_into_history(unsigned int fisrt, unsigned int n,
//				const struct fusion_cl_insn* insn){
//
//
//}


/*process constants (immediate/absolute)
 * and labels (jump targets/memory locations)*/
//static void process_label_constant(char* str, fusion_cl_insn* insn){
//	char* saved_input_line_pointer;
//	int symbol_with_at = 0;
//	int symbol_with_s = 0;
//  int symbol_with_m = 0;
//  int symbol_with_l = 0;

//	argument *cur_arg = fusion_cl_insn->arg;
//	input_line_pointer = str;
//	expression(&insn->exp);

//} 


struct regname{
	const char* name;
	unsigned int	num;
};

enum reg_file{
	REGF_GPR,	/* General Purpose*/
	REGF_FPR,	/* Floating Point */
	REGF_SYS,	/* System */
	REGF_MAX
};

/*hash for registers*/
static struct hash_control* reg_names_hash = NULL;

#define ENCODE_REG_HASH(rf, n)\
			((void*)(uintptr_t)((n) * REGF_MAX + (rf) + 1))
#define DECODE_REG_FILE(hash) (((uintptr_t)(hash) - 1) % REGF_MAX)
#define DECODE_REG_NUM(hash) (((uintptr_t)(hash) - 1) / REGF_MAX)

static void hash_reg_name(enum reg_file regf, const char* name, unsigned n){
	void* hash = ENCODE_REG_HASH(regf, n);
	const char* retval = hash_insert(reg_names_hash, name, hash); 

	if(retval !=NULL){
		as_fatal(_("internal error: can't hash `%s': %s"), name, retval);
	}

}
static void hash_reg_names(enum reg_file regf, const char* const names[], unsigned n){
	unsigned i;
	for(i = 0; i < n; i++){
		hash_reg_name(regf, names[i], i);
	}

}

static unsigned int reg_lookup_internal(char* s, enum reg_file regf){
	struct regname* r = (struct regname*) hash_find(reg_names_hash, s);
	if( r == NULL || DECODE_REG_FILE(r) != regf)
		return -1;
	return DECODE_REG_NUM(r);
}
/*
static int reg_lookup(char** sptr, enum reg_file regf){
	char* s = *sptr;	
	char* end;
	int reg = -1;
	SKIP_SPACE_TABS(s);	
	s = *sptr;
	// syntax checking 
	if( (*s != '$') && ( isdigit(*(s)) ) ) { //determine if number register is done correctly
	   as_bad(_("number regsiters require '$': %s"), s);
	   ignore_rest_of_line();
	   return -1;
	} else if( (*s == '$') && ( !isdigit((char)  *(s+1)) ) ) { //determine if number register is done correctly
	   as_bad(_("'$' is for number registers only: %s"), s);
	   ignore_rest_of_line();
	   return -1;
	}
	end = s;
	while((*end != ' ') || (*end != ',') || (*end != '\t') || (*end != '\n') )
		++end;
	 if((reg = reg_lookup_internal(s, regf)) >= 0)
		 *sptr = end;

		
	return reg;
}
*/

/*For validating instruction*/
static bfd_boolean validate_fusion_insn(const struct fusion_opc_info_t *opc ATTRIBUTE_UNUSED){
		/*Fix later to actually check if setup right*/
	return TRUE;
}

struct percent_op_match{
	const char* str;
	bfd_reloc_code_real_type reloc;
};

void md_begin(void) {
	int i = 0;
	if(!bfd_set_arch_mach(stdoutput, bfd_arch_fusion, 0))
			as_warn(_("Couldn't set architecture and machine"));
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
			
		do{
			if(!validate_fusion_insn(&fusion_insn_all[i]))	{
				as_fatal(_("Assembler is broken. Instruction not valid"));
			}
			++i;

		}while(fusion_insn_all[i].name && !strcmp(fusion_insn_all[i].name, name));
	}
	reg_names_hash = hash_new();
	hash_reg_names(REGF_GPR, fusion_gpreg_name, 32);
	hash_reg_names(REGF_GPR, fusion_gpreg_num, 32);
//	hash_reg_names(REGF_SYS, fusion_spreg_name, 32);
//	#define DECLARE_SYSREG(name, num)	hash_reg_name(REGF_SYS, #name, num);
	record_alignment(text_section, 2);
}

static insn_t fusion_apply_const_reloc(bfd_reloc_code_real_type reloc_type,
								bfd_vma value){
	switch(reloc_type){
		case BFD_RELOC_32:
			return value;
		case BFD_RELOC_16_PCREL:
		case BFD_RELOC_16:
			return GET_IMM_LI(value);
		case BFD_RELOC_HI16_PCREL:
		case BFD_RELOC_FUSION_HI16:
			return GET_IMM_LI(value >> 16);
		case BFD_RELOC_8:
			return GEN_SYS_IMM(value);
		case BFD_RELOC_FUSION_STORE:
			return GEN_S_IMM( (value) ); //unsure why this happens
		case BFD_RELOC_FUSION_LOAD:
			return GEN_L_IMM(value);
		case BFD_RELOC_FUSION_12:
			return GEN_I_IMM(value);
		case BFD_RELOC_FUSION_21:
			return GEN_J_IMM(value);
		default:
			abort();
	}

}


/* output instruction
 * ip: instruction information
 * address_expr: operand of instruciton to be used with reloc_type
 * expansionp: true if instruction is part of macro expansion*/
static void append_insn(struct fusion_cl_insn* ip, expressionS* addr_expr,
		bfd_reloc_code_real_type reloc_type){

	dwarf2_emit_insn(0);
	if(reloc_type != BFD_RELOC_UNUSED){
		reloc_howto_type* howto;

		gas_assert(addr_expr);
/*		
		if( (reloc_type == BFD_RELOC_FUSION_14_PCREL) \
			|| (reloc_type == BFD_RELOC_FUSION_21_PCREL)){
			int j = reloc_type == BFD_RELOC_FUSION_21_PCREL;
			//int best_case = 4;//insn_length(ip->insn_word);
			//unsigned worst_case = 8;//relaxed_branch_length(NULL, NULL, 0);
			add_relaxed_insn(ip, 4, 8, 
							RELAX_BRANCH_ENCODE(j, 4),
							addr_expr->X_add_symbol,
							addr_expr->X_add_number);
			return;
		} else{ 
		*/
			howto = bfd_reloc_type_lookup(stdoutput, reloc_type);
			if(howto == NULL){
				as_bad(_("Unsupported Fusion-Core relocation number: %d"), reloc_type);
			}

			switch(reloc_type){
				case BFD_RELOC_FUSION_14_PCREL:
				case BFD_RELOC_FUSION_21_PCREL:
					ip->fixptr = fix_new_exp(ip->frag, ip->frag_offset,
							bfd_get_reloc_size(howto),
							addr_expr, FALSE, reloc_type);
					ip->fixptr->fx_tcbit = 0;
					break;
				default:
					ip->fixptr = fix_new_exp(ip->frag, ip->frag_offset,
							bfd_get_reloc_size(howto),
							addr_expr, FALSE, reloc_type);
					ip->fixptr->fx_tcbit = 0;
				break;
			}
		//	ip->fixptr = fix_new_exp(ip->frag, ip->frag_offset,
		//					bfd_get_reloc_size(howto),
		//					addr_expr, FALSE, reloc_type);
		//	ip->fixptr->fx_tcbit = 0;
	//	}
	}
	add_fixed_insn(ip);
	install_insn(ip);

}

/* Build instruction, useful for macro expansions
 * passed to pointer the count number of instructions created so far,
 * an expression, the name of the instruction to build,
 * the operands required, and coresponding arguments*/
static void macro_build(expressionS *ep, const char* name, const char* fmt, ...){
	const struct fusion_opc_info_t* mo;
	struct fusion_cl_insn insn;
	bfd_reloc_code_real_type r;
	va_list args;

	#ifdef DEBUG
		as_warn(_("In macro_build"));
	#endif
	va_start(args, fmt);
	r = BFD_RELOC_UNUSED;
	mo = (struct fusion_opc_info_t* ) hash_find(op_hash_ctrl, name);
	gas_assert(strcmp(name, mo->name) == 0); //make sure we found the right instruction

	#ifdef DEBUG
	as_warn(_("Macro name found: %s"), mo->name);
	#endif

	create_insn(&insn, mo);
	insn.insn_word = 0; //clear insn to make sure it doesn't have extra numbers
	/*this may change, but is copied off of riscv for now*/
	INSERT_OPERAND(GEN_OPC(mo->opc), insn);
	if((mo->opc) == OPC_LI) {
		INSERT_OPERAND(GEN_DSEL_LI(mo->index), insn);	
	} else if((mo->opc) == OPC_IMM) {
		INSERT_OPERAND(GEN_ALUOP(mo->index), insn);	
	} else if((mo->opc) == OPC_LD) {
		INSERT_OPERAND(GEN_FUNCT_L(mo->index), insn);	
	} else if((mo->opc) == OPC_ST) {
		INSERT_OPERAND(GEN_FUNCT_S(mo->index), insn);	
	}
	for(;;){
		#ifdef DEBUG
		as_warn(_("Current argument: %c"), *fmt);
		#endif
		switch(*fmt++){
			case 'd':
				INSERT_OPERAND( GEN_RD( va_arg(args, int) ) , insn );
//				as_warn(_("Put register rd into insn"));
				continue;
			case 'a':
				INSERT_OPERAND( GEN_RSA( va_arg(args, int) ) , insn );
//				as_warn(_("Put register rsa into insn"));
				continue;
			case 'b':
//				as_warn(_("Put register rsb into insn"));
				INSERT_OPERAND( GEN_RSB( va_arg(args, int) ) , insn );
				continue;
			case 's': //small immediate
//				INSERT_OPERAND( GEN_I_IMM( va_arg(args, int) ), insn);
//				continue;
			case 'i': //load immedate/lower 16
			case 'u': //load immediate/upper 16
			case 'j': //jump immediate
				gas_assert(ep!= NULL);
				r = va_arg(args, int);
				continue;
			case '\0':
				break;
			case ',':
				continue;
			default:
				as_fatal(_("internal error: invalid macro. nice one"));
		}	
		break;
	}

	va_end(args);
	gas_assert(r == BFD_RELOC_UNUSED ? ep == NULL : ep != NULL);
#ifdef DEBUG
	as_warn(_("Macro instruction binary: %08lx"),(unsigned long) (insn.insn_word) );
#endif
	append_insn(&insn, ep, r);
}




/*sign-extend 32-bit constants that have bit 31 set, and all high
 * unset*/
static void normalize_constant_expr(expressionS *ex){
	if((ex->X_op == O_constant || ex->X_op == O_symbol)
		&& IS_ZEXT_32BIT_NUM(ex->X_add_number))
	  ex->X_add_number = (((ex->X_add_number & 0xffffffff) ^ 0x80000000)
							-0x80000000);
}
/*fail if expression is not constant*/
static void check_absolute_expr(struct fusion_cl_insn* ip, expressionS* ex){
	if(ex->X_op == O_big)
		as_bad(_("unsupported large constand; dat num too numb"));
	else if(ex->X_op != O_constant)
		as_bad(_("instruction %s requires absolute expression; get that outa here"),
						ip->insn_mo->name);
	normalize_constant_expr(ex);
}
/*
static void load_register(int reg, expressionS* ep, int dlb){
//	int freg;
//	expressionS hi32, lo32;
	if(ep->X_op != O_big){
		gas_assert(ep->X_op == O_constant);
		if( !dlb )
			normalize_constant_expr(ep);
		if( (ep->X_add_number >= 0) && (ep->X_add_number < 0x1000)){
			macro_build(ep, "ori", "d,a,i", reg, reg, BFD_RELOC_FUSION_12);
			return;
		} else if(IS_SEXT_16BIT_NUM(ep->X_add_number)){
			macro_build(ep, "li", "d,i", reg, BFD_RELOC_16);
			return;
		} else if((IS_SEXT_32BIT_NUM(ep->X_add_number))) {
			macro_build(ep, "lui", "d,i", reg, BFD_RELOC_FUSION_HI16);
			if((ep->X_add_number & 0xffff) != 0)
				macro_build(ep, "li", "d,i", reg, BFD_RELOC_16);
			return;
		}
	}
	if( (!dlb) ){//|| GPR_SIZE == 32) 32 bit values only so far
		char value[32];	
		sprintf_vma(value, ep->X_add_number);
		as_bad(_("number(0x%s) larger than 32 bits"), value);
		macro_build(ep, "li", "d,i", reg, BFD_RELOC_16);
		return;
	}

	if(ep->X_op != O_big){
		hi32 = *ep;
		hi32.X_add_number = (valueT) hi32.X_add_number >> 16;
		hi32.X_add_number = (valueT) hi32.X_add_number >> 16;
		hi32.X_add_number &= 0xffffffff;
		lo32 = *ep;
		lo32.X_add_number &= 0xffffffff;
	} else {
		gas_assert(ep->X_add_number > 2);
		if(ep->X_add_number == 3)
			generic_bignum[3] = 3;
		else if(ep->X_add_number > 3)
			as_bad(_("number larger than 64 bits"));
		lo32.X_op = O_constant;
		lo32.X_add_number = generic_bignum[0] + (generic_bignum[1] << 16);
		hi32.X_op = O_constant;
		hi32.X_add_number = generic_bignum[2] + (generic_bignum[3] << 16);
	}

	if(hi32.X_add_number == 0)
		freg = 0;
	else {
		int shift, bit;
	
	}

	abort();
}

static symbolS* make_internal_label(void){
	return (symbolS *) local_symbol_make(FAKE_LABEL_NAME, now_seg,
					(valueT) frag_now_fix(), frag_now);
}
*/
/*pc relative access*/
/*
static void pcrel_access(int destreg, int tempreg, expressionS *ep,
					const char* insn_name, const char* op_str,
					bfd_reloc_code_real_type hi_reloc,
					bfd_reloc_code_real_type lo_reloc){

		expressionS ep2;
		ep2.X_op = O_symbol;
		ep2.X_add_symbol = make_internal_label();
		ep2.X_add_number = 0;

		macro_build(ep, "lui", "d,u", tempreg, hi_reloc);
		macro_build(&ep2, insn_name, op_str, destreg, tempreg, lo_reloc);
		

	}
static void pcrel_load(int destreg, int tempreg, expressionS* ep,
				const char* insn_name, bfd_reloc_code_real_type hi_reloc,
				bfd_reloc_code_real_type lo_reloc){
	pcrel_access(destreg, tempreg, ep, insn_name, "d,s,j", hi_reloc,
							lo_reloc);
}
*/
static void fusion_call(int tmp, expressionS *ep){
	//load_address(tmp, ep);
	expressionS upper = *ep;
	expressionS lower = *ep;
	lower.X_add_number = (int32_t) (ep->X_add_number) & 0x0000ffff;
	upper.X_add_number -= lower.X_add_number;

	macro_build(&upper, "lui", "d,i", tmp, BFD_RELOC_FUSION_HI16);
//	as_warn(_("Finished building lui for fusion_call"));
	macro_build(&lower, "jrl", "a,j", tmp, BFD_RELOC_FUSION_21);  //can only be placed in RA0
//	as_warn(_("Finished building jrl for fusion_call"));

}

static void fusion_return( expressionS* ep){
	ep->X_add_number = 0x0; //don't want any offsets here; need one since it would throw a hissie fit otherwise
	macro_build(ep, "jr", "a,j", 4, BFD_RELOC_FUSION_21);
}

/*load integer constant into register */
static void load_const(int reg, expressionS* ep){
//	int shift = 16; 
	expressionS upper = *ep, lower = *ep;
	lower.X_add_number = ((uint32_t) ep->X_add_number) & 0xffff;// << (32-shift) >> (32-shift);
	upper.X_add_number -= lower.X_add_number;

	if(ep->X_op != O_constant){
		as_bad(_("unsupported large constant"));
		return;
	}
	
	int hi_reg = 0;
	if(upper.X_add_number != 0){
		// add lui insn
		macro_build(ep, "lui", "d, u", reg, BFD_RELOC_FUSION_HI16);

		hi_reg = reg;
	}

	if(lower.X_add_number != 0 || hi_reg == 0){
		//add li insn
		macro_build(ep, "li", "d, u", reg, BFD_RELOC_16); 
	}

}
static void load_address(int reg, expressionS* ep){
	if((ep->X_op != O_constant) && (ep->X_op != O_symbol)){
		as_bad(_("expression should be constant or symbol. can't knock on a door without an address. \n No loading in a red zone."));	
		ep->X_op = O_constant;
	}
	if(ep->X_op == O_constant){
		load_const(reg, ep);
		return;
	} else{
		if((IS_SEXT_32BIT_NUM(ep->X_add_number))) {
			macro_build(ep, "lui", "d,i", reg, BFD_RELOC_FUSION_HI16);
			macro_build(ep, "lni", "d,i", reg, BFD_RELOC_16);
			return;
		}
	} 
}

/*expand macro into one or more instructions*/
static void macro(struct fusion_cl_insn *ip, expressionS* imm_expr){//,
				//bfd_reloc_code_real_type* imm_reloc){
	int rd = GET_RD( (ip->insn_word));	
//	int rsa = GET_RSA( (ip->insn_word) );
//	int rsb = GET_RSB( (ip->insn_word) );
	int cpid = ip->insn_mo->cpid;
	int macro_id =(int) ip->insn_mo->index;
//	as_warn(_("In macro()"));
	if(cpid != CPID_MACRO){
		as_bad(_("Not a macro, broken assembler"));
	} else{
		switch(macro_id){
			case M_LA:
//				as_warn("Making la macro");
				load_address(rd, imm_expr);

				break;
			case M_CALL:
//				as_warn("Making call macro");
				fusion_call(22, imm_expr); //uses tmp0
				break;
			case M_RET:
//				as_warn("Making return macro");
				fusion_return( imm_expr); //uses ra
				break;
			default:

				as_bad(_("Macro %s not implemented"), ip->insn_mo->name);
				break;
		}	
	}
}
/*
static const struct percent_op_match percent_op_litype[] ={
	{"%hi",			BFD_RELOC_FUSION_HI16},
	{"%lo",			BFD_RELOC_16},
	{"%pcrel_hi", 	BFD_RELOC_HI16_PCREL},
	{"%pcrel_lo",	BFD_RELOC_16_PCREL},
	{0,0}
};
static const struct percent_op_match percent_op_stype[] ={
	{"%lo",			BFD_RELOC_FUSION_STORE},
	{0,0}
};
static const struct percent_op_match percent_op_ltype[] ={
	{"%lo",			BFD_RELOC_FUSION_LOAD},
	{0,0}
};
static const struct percent_op_match percent_op_btype[] ={
	{"%pcrel_lo",	BFD_RELOC_FUSION_14_PCREL},
	{0,0}
};
*/

/*Parse functions*/
/*
static bfd_boolean parse_relocation(char** str, bfd_reloc_code_real_type* reloc,
				const struct percent_op_match* percent_op){
	for( ; percent_op->str; percent_op++)
		if(strncasecmp(*str, percent_op->str, strlen(percent_op->str)) == 0){
			int len = strlen(percent_op->str);	
			if(!ISSPACE((*str)[len]) && (*str)[len] != '(')
				continue;
			*str += strlen(percent_op->str);
			*reloc = percent_op->reloc;

			if(*reloc != BFD_RELOC_UNUSED && !bfd_reloc_type_lookup(stdoutput, *reloc)){
				as_bad("relocation %s isn't supported currently", percent_op->str);
				*reloc = BFD_RELOC_UNUSED;
			}
			return TRUE;
		}
	return FALSE;


}
*/
static void getExpression(expressionS* ep, char* str){
	char* save_in;
	save_in = input_line_pointer;
	input_line_pointer = str;
	expression(ep);
	expr_end = input_line_pointer;
	input_line_pointer = save_in;

#ifdef DEBUG
			/*Printing out what kind of expression*/
		/*
			if(ep->X_op == O_constant){
				as_warn(_("Expression is O_constant"));
			} else if(ep->X_op == O_symbol){
				as_warn(_("Expression is O_symbol"));
			} else if(ep->X_op == O_register){
				as_warn(_("Expression is O_symbol"));
			} else if(ep->X_op == O_big){
				as_warn(_("Expression is O_big"));
			} else if(ep->X_op == O_illegal){
				as_warn(_("Expression is O_illegal"));
			}
		*/
#endif


}
/*
static size_t getSmallExpression(expressionS* ep, bfd_reloc_code_real_type* reloc,
				char* str, const struct percent_op_match *percent_op){
	size_t	reloc_index;
	unsigned crux_depth, str_depth;
	int regno;
	char* crux;
	
	//if(reg_lookup (&str, REGF_GPR, &regno)){
	regno = parse_register_operand(&str);
	if( regno != -1 ){
		ep->X_op = O_register;
		ep->X_add_number = regno;
		return 0;
	}
	reloc_index = -1;
	str_depth = 0;
	do {
		reloc_index++;
		crux = str;
		crux_depth = str_depth;
*/
			/*skip whitespace and brackets, keep count of bracket number*/
/*
		while(*str==' ' || *str == '\t' || *str == '(')
			if(*str++ == '(')
				str_depth++;
	}while(* str == '%' && reloc_index < 1 && parse_relocation(&str, reloc, percent_op));

	getExpression(ep, crux);
	str = expr_end;
	while(crux_depth > 0 && (*str == ')' || *str == ' ' || *str == '\t'))
		if(*str++ == ')')
			crux_depth--;
	if(crux_depth > 0)
		as_bad("missing ')'");
}
*/
/*Parse expression, then restore input line pointer*/

/*
static char* parse_exp_save_ilp(char *s, expressionS* op){
	char* save = input_line_pointer;
	input_line_pointer = s;
	expression(op);
	s = input_line_pointer;
	input_line_pointer = save;
	return s;
}
*/
/*Parse register for operands*/
static int parse_register_operand(char** ptr){
	 int reg = 0;
	 char* s = *ptr;
	 SKIP_SPACE_TABS(s);	
	if( (*s != '$') && ( isdigit(*(s+1)) || (*(s+1) == 'R') ) ) { //determine if number register is done correctly
	   as_bad(_("number regsiters require '$' or 'R': %s"), s);
	   ignore_rest_of_line();
	   return -1;
	} else if(  (*s == '$') && (isdigit(*(s+1))) ){
		if( isdigit(*(s+2)))
			reg = ((*(s+1)) - '0')*10; //get 10s place
		reg += (*(s+2)) - '0';
		if(reg > 31) {
			as_bad(_("Your register number is too damn high: %s"), s);
			return -1;
		}

		return reg;
	} else if(  (*s == '$') && (*(s+1) == 'R') ){
		if( isdigit(*(s+2)))
			reg = ((*(s+1)) - '0')*10; //get 10s place
		reg += (*(s+2)) - '0';
		if(reg > 31) {
			as_bad(_("Your register number is too damn high: %s"), s);
			return -1;
		}

		return reg;
	} else if( (*s == '$') && ( !isdigit((char)  *(s+1)) ) ) { //determine if number register is done correctly
	   as_bad(_("'$' is for number registers only: %s"), s);
	   ignore_rest_of_line();
	   return -1;
	}


//	 if(*s != '$') { //denote register with $ 
//		as_bad(_("expecting register, missing '$': %s"), s);
//		ignore_rest_of_line();
//		return -1;
//	 }
			
	//if zero register?
	 if(    (s[0] == 'z')
		 && (s[1] == 'e')
		 && (s[2] == 'r')
		 && (s[3] == 'o') ){
			if( !((s[4] == ' ') || (s[4] == '\t') || (s[4] == ',') || (s[4] == ')') || (s[4] == '\0') )){
				as_bad(_("not a real register: %s"), s);	
			}
		*ptr += 4; 
		return 0; //register 0
	 } else if( (s[0] == 's') && (s[1] == 'p')) {
			if( !((s[2] == ' ') || (s[2] == '\t') || (s[2] == ',') || (s[2] == '\0')) ){
				as_bad(_("not a real register: %s"), s);	
			}
			*ptr += 2;
			return 1;
	 } else if( (s[0] == 'f') && (s[1] == 'p')){
			if( !((s[2] == ' ') || (s[2] == '\t') || (s[2] == ',') || (s[2] == '\0') )){
				as_bad(_("not a real register: %s"), s);	
			}
			*ptr += 2;
			return 2;
	 } else if( (s[0] == 'g') && (s[1] == 'p')) {
			if(! ( (s[2] == ' ') || (s[2] == '\t') || (s[2] = ',') || (s[2] == '\0') )){
				as_bad(_("not a real register: %s"), s);	
				ignore_rest_of_line();
			}
			*ptr += 2;
			return 3;
	 } else if( (s[0] == 'r') && (s[1] == 'a')) {
			if( !((s[2] == ' ') || (s[2] == '\t') || (s[2] == ',') || (s[2] == '\0') )){
				as_bad(_("not a real register: %s"), s);	
			}
			*ptr += 2;
			return 4;
	 } else if( (s[0] == 'a') && (s[1] == 'r') && (s[2] == 'g')) {
			reg = s[3] - '0'; //get number value
			if( (reg < 0) || (reg > 3) ){ //not ARGX reg
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
			if( !((s[5] == ' ') || (s[5] == '\t') || (s[5] == ',') || (s[5] == '\0')) ){
				as_bad(_("not a real register: %s"), s);	
			}
		reg = s[4] - '0';
		if( (reg < 0) || (reg >1) ){
			as_bad(_("illegal return value regsiter"));
			ignore_rest_of_line();
			return -1;
		} else{
//			if(s[6] == '\0')
//				*ptr += 5;
			//else
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
			if( (s[2] == '1') && (s[3] == '0')){
				*ptr +=4;
				return 21; //gp10 is R21
			} else{
				*ptr += 3;
				return reg + 11;
			}
		//	if( s[3] == 1) { //need to check if 10
		//		reg = (s[4] - '0') + 10; 	
		//		if(s[4] != '0'){ //only option at the moment
		//			as_bad(_("illegal general use register"));	
		//			ignore_rest_of_line();
		//			return -1;
		//		} else {
		//			*ptr += 6;
		//			return 21; //gp10 is R21
		//		}
		//	} else {
		//		*ptr += 4;
		//		return reg + 11; //gp0 is R11
		//	}
		}
	 } else if ( (s[0] == 't') && (s[1] == 'm') 
				&& (s[2] == 'p')  ){
			if( !((s[4] == ' ') || (s[4] == '\t') || (s[4] == ',') || (s[4] == ')') || (s[4] == '\0')) ){
				as_bad(_("illegal temporary use register: %s"), s);	
			}
			reg = s[3] - '0'; 
		if( (reg < 0) || (reg > 8) ){
			as_bad(_("illegal temporary use register"));	
			ignore_rest_of_line();
			return -1;
		} else {
			*ptr += 4;
			return reg + 22; //tmp0 is R22
		}
			 
	 } else if( (s[0] == 'h') && (s[1] == 'i') && (s[2] == '0')){
			if( !((s[3] == ' ') || (s[3] == '\t') || (s[3] == ',') || (s[3] == '\0')) ){
				as_bad(_("not a real register: %s"), s);	
			}
		*ptr += 3;
		return 30; //HI0 is R30
	 } else if( (s[0] == 'l') && (s[1] == 'o') && (s[2] == 'w') && (s[3] == '0')){
			if( !((s[4] == ' ') || (s[4] == '\t') || (s[4] == ',') || (s[4] == '\0')) ){
				as_bad(_("not a real register: %s"), s);	
			}
		*ptr += 4;
		return 31; //LOW0 is R31
	 } else {
			as_bad(_("expecting register, unknown operand:%s"),*ptr);	
			ignore_rest_of_line();
			return -1; 
	 }

}

/*Operand Parsing functions; take various parameters in and spit out
 * the index number for the registers*/

int parse_rdab( int* rd, int* rsa, int* rsb, char* op_end){

	//skipping spaces
	while( (*op_end == ' ') || (*op_end == '\t') )
		   op_end++;	

	*rd = parse_register_operand(&op_end);
	if(*op_end != ','){
		as_warn(_("expecting comma deliminated registers"));
	}
	op_end++;

	//get rid of spaces and tabs
	while( (*op_end == ' ') || (*op_end == '\t'))
		op_end++;

	*rsa = parse_register_operand(&op_end);

	if(*op_end != ','){
		as_warn(_("expecting comma deliminated registers"));
	}
	op_end++;
	while( (*op_end == ' ') || (*op_end == '\t'))
		op_end++;

	*rsb = parse_register_operand(&op_end);
	if(*rsb == 4)
		return 0;
	while( (*op_end == ' ') || (*op_end == '\t') || !(*op_end == '\0'))
		op_end++;
	if(*op_end != '\0')
		as_warn(_("ignored rest of line: %s"), op_end);

	return 0;
} 

int parse_rda( int* rd, int* rsa, char* op_end){

	//skipping spaces
	while( (*op_end == ' ') || (*op_end == '\t') )
		   op_end++;	

	*rd = parse_register_operand(&op_end);
	if(*op_end != ','){
		as_warn(_("expecting comma deliminated registers"));
	}
	op_end++;

	//get rid of spaces and tabs
	while( (*op_end == ' ') || (*op_end == '\t'))
		op_end++;

	*rsa = parse_register_operand(&op_end);
			
	while( (*op_end == ' ') || (*op_end == '\t'))
		op_end++;
	if(*op_end != '\0')
		as_warn(_("ignored rest of line: %s"), op_end);

	return 0;
} 

int parse_rab( int* rsa, int* rsb, char* op_end){

	//skipping spaces
	while( (*op_end == ' ') || (*op_end == '\t') )
		   op_end++;	

	*rsa = parse_register_operand(&op_end);
	if(*op_end != ','){
		as_warn(_("expecting comma deliminated registers"));
	}
	op_end++;

	//get rid of spaces and tabs
	while( (*op_end == ' ') || (*op_end == '\t'))
		op_end++;

	*rsb = parse_register_operand(&op_end);
	if(*op_end != ','){
		as_warn(_("expecting comma deliminated operands"));
	}
	op_end++;
	while( (*op_end == ' ') || (*op_end == '\t'))
		op_end++;
	if(*op_end != '\0')
		as_warn(_("ignored rest of line: %s"), op_end);

	return 0;
} 

int parse_imm( int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip ATTRIBUTE_UNUSED){

	while( (**op_end == ' ') || (**op_end == '\t') )
		   (*op_end)++;
//	expressionS arg;
	//*op_end = parse_exp_save_ilp(*op_end, &arg);	
	//*op_end = getSmallExpression(imm_expr, imm_reloc, op_end);
//	if(reloc == BFD_RELOC_UNUSED){
		getExpression(imm_expr, *op_end);
//	} else {
		//getSmallExpression(imm_expr, &reloc, *op_end);
//	}
//	char* where;
//	where = frag_more(4);
//	as_warn(_("Insn parse: %s"), ip->insn_mo->name);
	switch(reloc){
			//finding if pc relative
		case BFD_RELOC_FUSION_14_PCREL:
				/*fallthru*/
		case BFD_RELOC_FUSION_21_PCREL:
		case BFD_RELOC_16_PCREL:
		case BFD_RELOC_HI16_PCREL:
//					ip->fixptr = fix_new_exp(frag_now,
//									(where - frag_now->fr_literal),
//									4,
//									imm_expr,
//									TRUE,
//									reloc);		
			break;
		case BFD_RELOC_32:
		case BFD_RELOC_16:
		case BFD_RELOC_FUSION_HI16:
		case BFD_RELOC_8:
		case BFD_RELOC_FUSION_LOAD:
		case BFD_RELOC_FUSION_STORE:
		case BFD_RELOC_FUSION_21:
		case BFD_RELOC_FUSION_12:
		if( imm_expr->X_op != O_constant ){
	//		as_fatal(_("Immediate is not constant: %lx"),(unsigned long)imm_expr->X_add_number);
	//		break;
		}
//					ip->fixptr = fix_new_exp(frag_now,
//									(where - frag_now->fr_literal),
//									4,
//									imm_expr,
//									FALSE,
//									reloc);		
			break;
		case BFD_RELOC_NONE:
			break;
		default:
			as_bad(_("not recognized relocation: %d"), reloc);
			abort();
	
	}	
	*imm = imm_expr->X_add_number;
	return 0;
} 

int parse_rdai( int* rd, int* rsa, int* imm,  char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip){

	//skipping spaces
	while( (**op_end == ' ') || (**op_end == '\t') )
		   (*op_end)++;	

	*rd = parse_register_operand(op_end);
	if(**op_end != ','){
		as_warn(_("expecting comma deliminated operands"));
	}
	(*op_end)++;

	//get rid of spaces and tabs
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;

	*rsa = parse_register_operand(op_end);
	if(**op_end != ','){
		as_warn(_("expecting comma deliminated operands"));
	}
	(*op_end)++;
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;

	parse_imm(imm, op_end, imm_expr, reloc, ip);
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;
//	if(**op_end != '\0')
//		as_warn(_("ignored rest of line: %s"), *op_end);
	return 0;
} 
int parse_rdi(int* rd, int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip){
	//skipping spaces
	while( (**op_end == ' ') || (**op_end == '\t') )
		   (*op_end)++;	

	*rd = parse_register_operand(op_end);
	if(**op_end != ','){
		as_warn(_("expecting comma deliminated registers"));
	}
	(*op_end)++;

	//get rid of spaces and tabs
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;
	parse_imm(imm, op_end, imm_expr, reloc, ip);
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;
//	if(**op_end != '\0')
//		as_warn(_("ignored rest of line: %s"), *op_end);
	return 0;

}
int parse_rai(int *rsa, int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip){
	//skipping spaces
	while( (**op_end == ' ') || (**op_end == '\t') )
		   (*op_end)++;	

	*rsa = parse_register_operand(op_end);
	if(**op_end != ','){
		as_warn(_("expecting comma deliminated registers"));
	}
	(*op_end)++;

	//get rid of spaces and tabs
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;
	parse_imm(imm, op_end, imm_expr, reloc, ip);
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;
//	if(**op_end != '\0')
//		as_warn(_("ignored rest of line: %s"), *op_end);
	return 0;
}
int parse_rabi(int* rsa, int* rsb, int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip){
	//skipping spaces
	while( (**op_end == ' ') || (**op_end == '\t') )
		   (*op_end)++;	

	*rsa = parse_register_operand(op_end);
	if(**op_end != ','){
		as_warn(_("expecting comma deliminated registers"));
	}
	(*op_end)++;

	*rsb = parse_register_operand(op_end);
	if(**op_end != ','){
		as_warn(_("expecting comma deliminated registers"));
	}
	(*op_end)++;

	//get rid of spaces and tabs
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;
	parse_imm(imm, op_end, imm_expr, reloc, ip);
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;
//	if(**op_end != '\0')
//		as_warn(_("ignored rest of line: %s"), *op_end);
	return 0;
}

int parse_rdai_offset( int* rd, int* rsa, int* imm,  char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip){

	//skipping spaces
	while( (**op_end == ' ') || (**op_end == '\t') )
		   (*op_end)++;	

	*rd = parse_register_operand(op_end);
	if(**op_end != ','){
		as_warn(_("expecting comma deliminated operands"));
	}
	(*op_end)++;

	//get rid of spaces and tabs
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;

	parse_imm(imm, op_end, imm_expr, reloc, ip);
	while( (**op_end != '(')){
		if(**op_end == '\0'){
			as_bad(_("expecting `(' before end: %s"), *op_end);
			return -1;
		}
		(*op_end)++;
	}
	(*op_end)++;
	*rsa = parse_register_operand(op_end);
	(*op_end)++;

	//SKIP_SPACE_TABS(op_end);

	//if(*op_end != ')'){
	//	as_bad(_("expecting `)' after register: %s"), op_end);
	//}
	while( (**op_end == ' ') || (**op_end == '\t') || (**op_end == ')'))
		(*op_end)++;

//	if(**op_end != '\0')
//		as_warn(_("ignored rest of line: %s"), *op_end);
	return 0;
} 

int parse_rai_offset(int *rsa, int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip){
	//get rid of spaces and tabs
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;

	parse_imm(imm, op_end, imm_expr, reloc, ip);
	if(ip->insn_mo->cpid != CPID_MACRO){ //GAS freaks out for macro instructions
	while( (**op_end != '(')){
		if(**op_end == '\0'){
			as_bad(_("expecting `(' before end"));
			return -1;
		}
		(*op_end)++;
	}
	(*op_end)++;
	
	*rsa = parse_register_operand(op_end);
	}
	(*op_end)++;
	
	while( (**op_end == ' ') || (**op_end == '\t') || (**op_end == ')'))
		(*op_end)++;
//	if(**op_end != ')'){
//		as_bad(_("expecting `)' after register: %s"), *op_end);
//	} else {
//		(*op_end)++;
//	}
//	if(**op_end != '\0')
//		as_warn(_("ignored rest of line: %s"), *op_end);
	return 0;
}
int parse_rabi_offset(int* rsa, int* rsb, int* imm, char** op_end, expressionS* imm_expr, bfd_reloc_code_real_type reloc, struct fusion_cl_insn* ip){
		//skipping spaces
	while( (**op_end == ' ') || (**op_end == '\t') )
		   (*op_end)++;	

	*rsb = parse_register_operand(op_end);
	if(**op_end != ','){
		as_warn(_("expecting comma deliminated operands"));
	}
	(*op_end)++;

	//get rid of spaces and tabs
	while( (**op_end == ' ') || (**op_end == '\t'))
		(*op_end)++;

	parse_imm(imm, op_end, imm_expr, reloc, ip);
	while( (**op_end != '(')){
		if(**op_end == '\0'){
			as_bad(_("expecting `(' before end"));
			return -1;
		}
		(*op_end)++;
	}
	(*op_end)++;
	*rsa = parse_register_operand(op_end);
	(*op_end)++;
//	if(*op_end != ')'){
//		as_bad(_("expecting `)' after register: %s"), op_end);
//	}
	while( (**op_end == ' ') || (**op_end == '\t') || (**op_end == ')'))
		(*op_end)++;

//	if(**op_end != '\0')
//		as_warn(_("ignored rest of line: %s"), *op_end);
	return 0;	
}


/*
static const char* fusion_ip(char *str, struct fusion_cl_insn* ip, 
	expressionS* imm_expr, bfd_reloc_code_real_type* imm_reloc){
	char* s;
	const char* args;
	char c = 0;
	struct fusion_opc_info_t* insn;
	char* argsStart;
	unsigned int regno;
	const struct percent_op_match *p;
	const char* errpor = "unrecognized instruction";



}
*/

/*determines relocation based off of opcode*/
bfd_reloc_code_real_type return_reloc(fusion_opc_info_t* insn, char* str){
	/*get opcode*/
	insn_t opc = insn->opc;
	insn_t dsel = insn->index; 	//for li insns
	insn_t usereg = insn->index; //for jump insns
	unsigned cpid_value = insn->cpid;
	bfd_reloc_code_real_type ret_reloc = BFD_RELOC_UNUSED;

	/*Determine which instruction kind*/
	switch(opc){
		/*Integer Instructions*/
		case OPC_INT:
			break;
		/*Immediate Instructions*/
		case OPC_IMM:
			ret_reloc = BFD_RELOC_FUSION_12;
			break;
		/*Load Instructions*/
		case OPC_LD:
			ret_reloc = BFD_RELOC_FUSION_LOAD;
			break;
		/*Store Instructions*/	
		case OPC_ST:
			ret_reloc = BFD_RELOC_FUSION_STORE;
			break;	
		/*Load Immeidate Instructions*/
		case OPC_LI:
		 //need to determine what kind of load immediate
			switch(dsel){
				case 3:  //lui
				case 4:  //lusi
				case 5:  //lugi
				case 11: //luni
				case 12: //lunsi
				case 13: //lungi
					ret_reloc = BFD_RELOC_FUSION_HI16;
					break;
				case 0:	 //li
				case 1:  //lsi
				case 2:  //lgi
				case 8:  //lni
				case 9:	 //lnsi
				case 10: //lngi
				default:
					ret_reloc = BFD_RELOC_16;
					break;
			}
			break;
		/*Jump Instruction*/
		case OPC_JMP:
			if(cpid_value == CPID_MACRO){
				ret_reloc = BFD_RELOC_FUSION_21;
				break;
			}
			switch(usereg){
				case 0:	
					ret_reloc = BFD_RELOC_FUSION_21_PCREL;
					break;
				case 1:
					ret_reloc = BFD_RELOC_FUSION_21;
					break;
				default:
					as_fatal(_("What did you do?? That jump insn can't have a third option in whether it uses regs or not"));
					break;
			}
			break;
		/*Jump Link Instruction*/
		case OPC_JLNK:
			if(cpid_value == CPID_MACRO){
				ret_reloc = BFD_RELOC_FUSION_21;
				break;
			}
			switch(usereg){
				case 0:	
					ret_reloc = BFD_RELOC_FUSION_21_PCREL;
					break;
				case 1:
					ret_reloc = BFD_RELOC_FUSION_21;
					break;
				default:
					as_fatal(_("What did you do?? That jump insn can't have a third option in whether it uses regs or not"));
					break;
			}
			break;
		/*Branch Instructions*/
		case OPC_BRANCH:
			ret_reloc = BFD_RELOC_FUSION_14_PCREL;
			break;
		/*System Instrucitons*/
		case OPC_SYS:
			if(insn->imm_mask == MASK_NO_IMM) //if no imm, no reloc
					break;
			ret_reloc = BFD_RELOC_8;
			break;
		case 0x00:
			break;
		/*Unknown Instructions*/
		default:
			as_bad(_("Unknown opcode, how did you manage that?: %s"), str);
			break;
			
	}
	return ret_reloc;
}

/*Turns instruction into binary
 * p: relocation info things
 * str: input from stream for getting assembly info
 * insn: struct values to grab
 * insn_bin: binary value to get
 * returns FALSE if can't assemble*/
//bfd_boolean assemble_insn_bin(char* str, fusion_opc_info_t* insn, insn_t* insn_bin){
bfd_boolean assemble_insn_bin(char* str, struct fusion_cl_insn* ip, expressionS* imm_expr,
						bfd_reloc_code_real_type* imm_reloc){

	fusion_opc_info_t* insn = (fusion_opc_info_t*)ip->insn_mo;
	imm_expr->X_op = O_absent;
	*imm_reloc = BFD_RELOC_UNUSED;

	unsigned cpid_value = insn->cpid; //get CPID value
	insn_t opc = insn-> opc; //get opcode
	unsigned args;
	char* op_start = str;
	char* op_end = str;
//	char* p = ;
	/*Get relocation based off of instruction opcode*/
	bfd_reloc_code_real_type reloc = return_reloc( insn, str );
	*imm_reloc = reloc;
	if( (cpid_value) > CPID_MAX){
		as_bad(_("Unknown co-processor instruction: %s"), op_start);
		return FALSE;
	} else if((cpid_value == CPID_MAIN) || (cpid_value == CPID_MACRO)){ //if for main processor
		
		args = insn->args; //operands for instruction	

		if( (args > MAX_USE_OP) ){
			as_bad(_("Incorrect arguments, what did you do? insn: %s"), op_start);
		return FALSE;
	}


			
	//operand values
	int Rd = 0;  //Destination Register
	int RSa = 0; //Source A register
	int RSb = 0; //Source B register
	int imm = 0; //Immediate value
	

	insn_t imm_mask = insn->imm_mask;	
	if(imm_mask == MASK_NO_IMM){
		/*Get Operands*/
		switch(args){
			case USE_NONE:
				while( (*op_end == ' ') || (*op_end == '\t'))
					op_end++;
				if(*op_end != '\0')
					as_warn(_("ignored rest of line: %s"), op_end);
				break;
			
			case USE_RDAB:
				parse_rdab( &Rd, &RSa, &RSb, op_end);
				break;

			case USE_RDA:
				parse_rda( &Rd, &RSa, op_end);
				break;

			case USE_RAB:
				parse_rab( &RSa, &RSb, op_end);
				break;
			default:
				break;

				
		} 
		if( (Rd == -1) || (RSa == -1) || (RSb == -1) ){
			as_bad(_("unknown register used %s"), op_start);	
			return FALSE;
		}

	} else { //instructions use immediate values
			switch(args){
				case USE_RDAI:
					parse_rdai( &Rd, &RSa, &imm, &op_end, imm_expr, reloc, ip);
					break;	
				case USE_RDI:
					parse_rdi( &Rd, &imm, &op_end, imm_expr, reloc, ip);
					break;
				case USE_RAI:
					parse_rai( &RSa, &imm, &op_end, imm_expr, reloc, ip);
					break;
				case USE_RABI:
					parse_rabi( &RSa, &RSb, &imm, &op_end, imm_expr, reloc, ip);
					break;
				case USE_RDAI_O:
					parse_rdai_offset( &Rd, &RSa, &imm, &op_end, imm_expr, reloc, ip);
					break;
				case USE_RAI_O:
					parse_rai_offset( &RSa, &imm, &op_end, imm_expr, reloc, ip);
					break;
				case USE_RABI_O:
					parse_rabi_offset( &RSa, &RSb, &imm, &op_end, imm_expr, reloc, ip);
					break;
				case USE_I:
					parse_imm( &imm, &op_end, imm_expr, reloc, ip);
					while( (*op_end == ' ') || (*op_end == '\t'))
						op_end++;
					if(*op_end != '\0')
						//as_warn(_("ignored rest of line: %s"), op_end);
					break;	
			}	
	}


	/*Temporary immediate value, until relocatations are finalized*/
//	int imm_tmp = 0;//0xdeadbeef;

	/*Determine which instruction kind*/
	switch(opc){
		/*Integer Instructions*/
		case OPC_INT:
	//		imm_expr->X_op = O_absent;
			ip->insn_word = MAKE_R_TYPE(Rd, RSa, RSb, 0, insn->index );
			break;
		/*Immediate Instructions*/
		case OPC_IMM:
			check_absolute_expr(ip, imm_expr);
	//		imm_expr->X_op = O_constant;
			ip->insn_word = MAKE_I_TYPE(Rd, RSa, imm , insn->index);
			break;
		/*Load Instructions*/
		case OPC_LD:
			ip->insn_word = MAKE_L_TYPE(Rd, RSa, insn->index, imm);
			break;
		/*Store Instructions*/	
		case OPC_ST:
			 ip->insn_word = MAKE_S_TYPE(insn->index, RSa, RSb, imm);
			break;	
		/*Load Immeidate Instructions*/
		case OPC_LI:
					//check_absolute_expr(ip, imm_expr);
					ip->insn_word = MAKE_LI_TYPE(Rd, insn->index, imm);
					break;
				/*Jump Instruction*/
				case OPC_JMP:
					if(RSa != 0)
						*imm_reloc = BFD_RELOC_FUSION_21;
					ip->insn_word = MAKE_J_TYPE(RSa, imm);
					break;
				/*Jump Link Instruction*/
				case OPC_JLNK:
					if(RSa != 0)
						*imm_reloc = BFD_RELOC_FUSION_21;
					ip->insn_word = MAKE_JL_TYPE(RSa, imm);
					break;
				/*Branch Instructions*/
				case OPC_BRANCH:
					ip->insn_word = MAKE_B_TYPE(RSa, RSb, imm, insn->index);
					break;
				/*System Instrucitons*/
				case OPC_SYS:
					ip->insn_word = MAKE_SYS_TYPE(Rd, RSa, insn->index, imm);
					break;
				case 0x00:
					ip->insn_word = 0x00000000; //since nop
					break;
				/*Unknown Instructions*/
				default:
					as_bad(_("Unknown opcode, \
						how did you manage that?: %s"), op_start);
					ip->insn_word = 0x00000000;
			break;
			
	}
	} else{
		as_bad (_("No co-processor instructions exist yet, don't know \
					what this is: %s"), op_start);
		return FALSE;
	}

	return TRUE;
}



void md_assemble(char* str){
	char* op_start;
	char* op_end;


	fusion_opc_info_t *insn;
	struct fusion_cl_insn ip;
	//char* p;
	char pend;
				
	//insn_t iword = 0;
	int nlen = 0;

	SKIP_SPACE_TABS(str);

	op_start = str;
	expressionS imm_expr;
	bfd_reloc_code_real_type imm_reloc;

	/* Finding op code end*/
	for(op_end = str; *op_end && !is_end_of_line[*op_end & 0xff] && *op_end != ' '; op_end++)
		nlen++;

	pend = *op_end;
	*op_end = 0;

	if(nlen == 0){
		as_bad (_("couldn't find instruction"));
	}
	insn = (fusion_opc_info_t *)hash_find(op_hash_ctrl, op_start);
	/*fix hash if incorrect instruction found; happens for jumps?*/
	*op_end = pend;
	if(insn == NULL){
		as_bad(_("Unknown instruction: %s"), op_start);
		return;
	}
	create_insn(&ip, insn);
	bfd_boolean assemble_success = assemble_insn_bin(op_end, &ip, &imm_expr, &imm_reloc);
	/*Make sure assembly is done properly*/	
	if( !assemble_success ){
		as_bad(_("Couldn't assemble instruction:%s"), op_start);
		return;
	}
	if(ip.insn_mo->cpid == CPID_MACRO){
		macro(&ip, &imm_expr);//, &imm_reloc);
	} else if(ip.insn_mo->cpid == CPID_MAIN){
		append_insn(&ip, &imm_expr, imm_reloc);
	}else{
		as_fatal(_("Unimplemented CPID"));
		return;
	}


//	md_number_to_chars(p, iword, 4);
//	dwarf2_emit_insn(4);
//	while(ISSPACE(*op_end))
//			op_end++;
//	if(*op_end != 0)
//		as_warn(_("rest of line ignored; clean yo shit"));
	if(pending_reloc)
		as_warn(_("something forgot to clean up ooooooh\n"));
	return;	
}


const char* md_atof(int type, char* litP, int* sizeP){
	return ieee_md_atof(type, litP, sizeP, target_big_endian);
}

const char *md_shortopts ="";

struct option md_longopts[] = {
	{NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof(md_longopts);

int md_parse_option(int c ATTRIBUTE_UNUSED, const char* arg ATTRIBUTE_UNUSED){
	/*no options at the moment*/
	return 1;

}

void md_show_usage(FILE* stream ATTRIBUTE_UNUSED){
	fprintf(stream, _("\nNo options available at the moment\n"));
}

/* Debug information additions */
void fusion_cfi_frame_initial_instructions(void){
	cfi_add_CFA_def_cfa_register (X_SP);
}

int tc_fusion_regname_to_dw2regnum( char *regname){
	int reg; 

	if(( reg = reg_lookup_internal (regname, REGF_GPR)) >= 0)
		return reg;
//	if(( reg = reg_lookup_internal (regname, REGF_FPR)) >= 0)
//		return reg + 32;
//	if(( reg = reg_lookup_internal (regname, REGF_SYS)) >= 0)
//		return reg + 64;
	as_bad (_("unknown register `%s'"), regname);
	return -1;

}

void md_apply_fix(fixS *fixP, valueT* valP, segT seg ATTRIBUTE_UNUSED){
	bfd_byte *buf =(bfd_byte*)( fixP->fx_where + fixP->fx_frag->fr_literal );
	fixP->fx_addnumber = *valP;
	switch(fixP->fx_r_type){
		case BFD_RELOC_32:
		case BFD_RELOC_8:
		case BFD_RELOC_FUSION_HI16:
		case BFD_RELOC_16:
		case BFD_RELOC_FUSION_STORE:
		case BFD_RELOC_FUSION_LOAD:
#ifdef DEBUG
		as_warn(_("Const_reloc: %08lx"), (unsigned long int)fusion_apply_const_reloc(fixP->fx_r_type, *valP));
#endif
			bfd_putb32( fusion_apply_const_reloc(fixP->fx_r_type, *valP) | bfd_getb32(buf), buf);
			if(fixP->fx_addsy == NULL)
				fixP->fx_done = TRUE;
			break;
		case BFD_RELOC_FUSION_12:
			bfd_putb32( (fusion_apply_const_reloc(fixP->fx_r_type, *valP) ) | bfd_getb32(buf), buf);
			if(fixP->fx_addsy == NULL)
				fixP->fx_done = TRUE;
			break;
		case BFD_RELOC_FUSION_14_PCREL:
			if(!*valP)
				break;
			 if(fixP->fx_addsy){
				 bfd_vma target = (S_GET_VALUE(fixP->fx_addsy) + *valP);
				 bfd_vma delta = (target - md_pcrel_from(fixP));
#ifdef DEBUG
				 as_warn(_("Target: %lx"), (unsigned long int)target);
				 as_warn(_("Delta: %lx"), (unsigned long int)delta);
				 as_warn(_("md_pcrel_from(fixP): %lx"), (unsigned long int)md_pcrel_from(fixP));
#endif
				 if ( ( ( (signed int) delta ) < -8192) || ( ( (signed int) delta ) > 8191)){
					 as_bad_where(fixP->fx_file, fixP->fx_line,_("too large pc relative branch"));
				 }
				 bfd_putb32( bfd_getb32(buf) | GEN_B_IMM(delta), buf);
			 }
			break;

		case BFD_RELOC_FUSION_21_PCREL:
			if(!*valP)
			break;
			if(fixP->fx_addsy){
				bfd_vma target = (S_GET_VALUE(fixP->fx_addsy) + *valP);
				bfd_vma delta = (target - md_pcrel_from(fixP));
#ifdef DEBUG
				as_warn(_("Target: %x"), (unsigned int)target);
				as_warn(_("Delta: %x"), (unsigned int)delta);
			 	as_warn(_("md_pcrel_from(fixP): %lx"), (unsigned long int)md_pcrel_from(fixP));
#endif
				if ( ( ( (signed int) delta ) < -1048576) || ( ( (signed int) delta ) > 1048576)) {
				 	as_bad_where(fixP->fx_file, fixP->fx_line,_("too large pc relative jump"));
				}
				bfd_putb32( bfd_getb32(buf) | (GEN_J_IMM(delta)), buf);				
			}
			break;
		case BFD_RELOC_16_PCREL:
		case BFD_RELOC_HI16_PCREL:
			if(!*valP)
			break;
			if(fixP->fx_addsy){
				bfd_vma target = (S_GET_VALUE(fixP->fx_addsy) + *valP);
				bfd_vma delta = (target - md_pcrel_from(fixP));
#ifdef DEBUG
				as_warn(_("Target: %x"), (unsigned int)target);
				as_warn(_("Delta: %x"), (unsigned int)delta);
			 	as_warn(_("md_pcrel_from(fixP): %lx"), (unsigned long int)md_pcrel_from(fixP));
#endif
				if ( ( ( (signed int) delta ) < -32768) || ( ( (signed int) delta ) > 32767)) {
				 	as_bad_where(fixP->fx_file, fixP->fx_line,_("too large pc relative load. No loading in a white zone"));
					
				}
				bfd_putb32( bfd_getb32(buf) | (GEN_LI_IMM(delta)), buf);				
			}
			break;
		case BFD_RELOC_FUSION_21:
			if(!*valP)
				break;
			if(fixP->fx_addsy != NULL){
				bfd_vma target = (S_GET_VALUE(fixP->fx_addsy) + *valP);
#ifdef DEBUG
				as_warn(_("using addsy; jr offset"));
				as_warn(_("Target: %08lx"), (unsigned long)target);
				as_warn(_("Imm value: %08lx"), (unsigned long)GEN_J_IMM(target));
#endif
				if( ((unsigned long) target) > 0x001fffff){
					as_bad_where(fixP->fx_file, fixP->fx_line,_("too large jump offset"));
				}
				bfd_putb32( (bfd_getb32(buf) ) | (GEN_J_IMM(target) ), buf );
			} else {
#ifdef DEBUG
				as_warn(_("using constant reloc; jr offset"));
#endif
				bfd_putb32( fusion_apply_const_reloc(fixP->fx_r_type, *valP) | bfd_getb32(buf), buf);
				fixP->fx_done = TRUE;
			}
			break;
		default:
			as_fatal(_("Unknown reloc code: %d. Please report this to the devs"), (unsigned int) fixP->fx_r_type);
			//abort();
	if(fixP->fx_subsy != NULL){
		as_bad_where(fixP->fx_file, fixP->fx_line,
						_("unsupported symbol subtraction"));
	}
	if( (fixP->fx_addsy == NULL) && fixP->fx_pcrel == 0)
			fixP->fx_done = 1;
	
	}
}

arelent *tc_gen_reloc(asection* section ATTRIBUTE_UNUSED, fixS *fixp){
	arelent* reloc;

	reloc = (arelent *)xmalloc( sizeof(arelent) );
	reloc->sym_ptr_ptr = (asymbol **) xmalloc( sizeof( asymbol* ) );
	*reloc->sym_ptr_ptr= symbol_get_bfdsym(fixp->fx_addsy);
	if(fixp->fx_r_type ==  BFD_RELOC_FUSION_21_PCREL) {
		bfd_vma delta = (fixp->fx_frag->fr_address + fixp->fx_addnumber);
		if ( ( ( (signed int) delta ) < -1048576) || ( ( (signed int) delta ) > 1048575)) {
				 	as_bad_where(fixp->fx_file, fixp->fx_line,_("too large pc relative jump"));
		} else{
			reloc->addend = GEN_J_IMM( (delta ) ) ;
			reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
		}
	}	else if (fixp->fx_r_type ==  BFD_RELOC_FUSION_14_PCREL){
			bfd_vma delta = (fixp->fx_frag->fr_address + fixp->fx_addnumber);
			if ( ( ( (signed int) delta ) < -8192) || ( ( (signed int) delta ) > 8191)){
				as_bad_where(fixp->fx_file, fixp->fx_line,_("too large pc relative branch")); 
	 		} else {
				reloc->addend = delta;//GEN_B_IMM(( (delta ) ) );
				reloc->address = fixp->fx_frag->fr_address + fixp->fx_where; 
			}
	} else if(fixp->fx_r_type == BFD_RELOC_FUSION_21 ) {
#ifdef DEBUG
		as_warn(_("fx_addnumber: %08lx"),(unsigned long int) fixp->fx_addnumber);
		as_warn(_("GEN_J_IMM(fx_addnumber): %08lx"),(unsigned long int) GEN_J_IMM(fixp->fx_addnumber));
#endif
		reloc->addend  = GEN_J_IMM(fixp->fx_addnumber);
		reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
	} else {
		reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
		reloc->addend = fixp->fx_addnumber;
	}
#ifdef DEBUG
	as_warn(_("Reloc address: %x"), (unsigned int) (reloc->address));
#endif
	reloc->howto = bfd_reloc_type_lookup(stdoutput, fixp->fx_r_type);

	if(reloc->howto == NULL){
		as_bad_where(fixp->fx_file, fixp->fx_line, _("Cannot represent relocation type %s"),bfd_get_reloc_code_name(fixp->fx_r_type));
		return NULL;
	}
	return reloc;
}

long md_pcrel_from(fixS* fixP){
	return (fixP->fx_where + fixP->fx_frag->fr_address);
}
/*
long md_pcrel_from(fixS *fixP){
	valueT addr = fixP->fx_where + fixP->fx_frag->fr_address;
	switch(fixP->fx_r_type){
		case BFD_RELOC_32:
			return addr + 4;
		case BFD_RELOC_FUSION_14_PCREL:
		*/
			/*offset from end of insn*/
/*			return addr + 2 ;
		case BFD_RELOC_FUSION_21_PCREL:
			return addr + 3;
		default:
			abort();
			return addr;
	}

}
*/
void md_number_to_chars(char* ptr, valueT use, int nbytes){
	number_to_chars_bigendian(ptr, use, nbytes);
}
/*
static valueT md_chars_to_number(char* buf, int n){
	valueT result = 0;
	unsigned char* where = (unsigned char* ) buf;
	while(n--){
		result <<= 0x08;
		result |= (*where++& 0xff);
	}	
	return result;
}
*/

static void s_bss(int ignore ATTRIBUTE_UNUSED){
	subseg_set(bss_section, 0);
	demand_empty_rest_of_line();
}

/* Pseduo-op table*/

static const pseudo_typeS fusion_pseudo_table[] ={
	{"byte",	cons,		1},
	{"half",	cons,		2},
	{"word",	cons,		4},
	{"bss",		s_bss,		0},
//	{"align",	do_align,	0},
	{NULL,		NULL,		0},
};

void fusion_pop_insert(void){
	extern void pop_insert(const pseudo_typeS*);

	pop_insert(fusion_pseudo_table);
}
