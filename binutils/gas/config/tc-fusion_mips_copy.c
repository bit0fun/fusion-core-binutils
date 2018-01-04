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




#include "as.h"
#include "config.h"
#include "safe-ctype.h"
#include "opcode/fusion.h"
#include "opcode/fusion-opc.h"
#include "elf/fusion.h"
#include <stdint.h>



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


static struct hash_control *op_hash_ctrl;;

const pseudo_typeS md_pseudo_table[] = {
	
	{"byte", s_cons, 0},}
	{"half", s_cons, 2},
	{"word", s_cons, 4},
	{"gpword", s_gpword, 0 },
	{"gpvalue", s_gpvalue, 0},
	{"bss", s_change_sec, 'b'},
	{"align", s_align, 0},
	{"global", s_fusion_globl, 0},
	{"globl", s_fusion_globl, 0},
	{"text", s_change_sec, 	't'},
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

int fusion_pop_insert(void){
	pop_insert(md_pseudo_table);
}

/*linked list of symbols labeling the current instruction*/
struct insn_label_list{
	struct insn_label_list* next;
	symbolS* label;
};

static struct insn_label_list* free_insn_labels;
#define label_list tc_segment_info_data.labels

static inline void fusion_clear_insn_labels(void) {
	struct insn_label_list **pl;
	segment_info_type *si;
	if(now_seg){
		for(pl = &free_insn_labels; *pl != NULL; pl = &(*pl)->next)
			;
		si = seg_info(now_seg);
		*pl = si->label_list;
		si->label_list = NULL;
	}
}

static char* expr_end;

/* expression in macro instruction; set by fusion_ip 
 * when populated, it is always O_constant */

static expressionS imm_expr;

/* relocatable field is the instruction and relocations associated
 * used for offsets, and address operands in macros */

static expressionS offset_expr;
static bfd_reloc_code_real_type offset_reloc[3] = 
	{BFD_RELOC_UNUSED, BFD_RELOC_UNUSED, BFD_RELOC_UNUSED};

/* denotes if we are currently assembling an instruction*/
static bfd_boolean fusion_assembling_insn;

/* returns instrution length */
static inline unsigned int insn_length( const struct fusion_cl_insn* insn ){
	/* this may change for support for various new co-processors */
	return 4;
}



/*what to do to output*/
enum append_method{
	/*Normal append instruction*/
	APPEND_NORMAL,
	/*Append normally, with NOP afterwards*/
	APPEND_NOP,
	/*Insert instruction before last one*/
//	APPEND_SWP

}

extern int target_big_endian;

/*error formats*/
enum fusion_insn_error_format{
	ERR_FMT_PLAIN,
	ERR_FMT_I,
	ERR_FMT_SS,
};

/* info about error that was found while assembling current insn  */
struct fusion_insn_error{
	int min_argnum;
	enum fusion_insn_error_format format;
	const char* msg;
	union{
		int i;
		const char* ss[2];
	} u;
};

/* the actual error for current instruction*/
static struct fusion_insn_error insn_error;

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


/* Handle of the OPCODE hash table*/
static struct hash_control *op_hash = NULL;
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

/* read instruction from buf */
static unsigned long read_insn(char* buf){
	return bfd_getb32 ((bfd_byte *) buf);
}

/* write instruction to buf */
static char* write_insn(char* buf, unsigned int insn){
	md_number_to_chars(buf, insn, 4);
	return buf+4;
}

/* Puts instruction at the location specified by frag, and frag_offset*/
static void install_insn(const struct fusion_cl_insn *insn){
	char *f = insn->frag->fr_literal + insn->frag_offset;
//	md_number_to_chars(f, insn->insn_opc, 4); //4 since 32 bit instruction
	write_insn(f, insn->insn_opcode);

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
static void add_relaxed_insn(struct fusion_cl_insn* insn, int max_chars,
		int var, relax_substateT subtype, symbolS* symbol, 
		offsetT offset){
	frag_grow(max_chars);
	move_insn(insn, frag_now, frag_more(0) - frag_now->fr_literal);
	insn->fixed_p = 1;
	frag_var(rs_machine_dependent, max_chars, var, subsype, symbol, offset,
			NULL);

}

//not using at the moment

/*insert N copies of INSN into history buffer, starting at first.
 * netierh first nor n need to be clipped*/
//static void insert_into_history(unsigned int fisrt, unsigned int n,
//				const struct fusion_cl_insn* insn){
//
//
//}

/* clear error in insn_error  */
static void clear_insn_error(void){
	memset(&insn_error, 0, sizeof(insn_error));
}

/*possibly record error message, msg, for current instruction
 * if error is about argument, argnum, is 1 otherwise 0
 * format is the format of the message. return true if used, 
 * false if kept*/
static bfd_boolean set_insn_error_format(int argnum, 
			enum fusion_insn_error_format format, const char* msg){
	if(argnum == 0){
		if(insn_error.msg)
			return FALSE;
	} else {
		if(argnum < insn_error.min_argnum)
			return FALSE;
		if(argnum == insn_error.min_argnum
				&& insn_error.msg
				&& strcmp(insn_error.msg, msg) !=0){
			insn_error.msg = 0;
			insn_error.min_argnum += 1;
			return FALSE;
		}
	}
	insn_error.min_argnum = argnum;
	insn_error.format = format;
	insn_error.msg = msg;
	return TRUE;
}

/* record an instruction error wirth no % foramt fields
 * argnum and msg are for set_insn_error_format*/
static void set_insn_error(int argnum, const char* msg){
	set_insn_error_format(argnum, ERR_FMT_PLAIN, msg);
}

/* record instruction error with one %d field, I.
 * argnum and msg are for set_insn_error_format */
static void set_insn_error_i (int argnum, const char* msg, int i){
	if(set_insn_error_format(argnum, ERR_FMT_T, msg)){
		insn_error.u.i = i;
	}
}

/* record instruction error with two %s fileds, s1, s2
 * argnum and msg are for set_insn_error_format */
static void set_insn_error_ss(int argnum, const char* msg, const char* s1,
		const char* s2){
	if(set_insn_error_format(argnum, ERR_FMT_SS, msg)){
		insn_error.u.ss[0] = s1;
		insn_error.u.ss[1] = s2;
	}
}

/* report the error in insn_error, which is against assembly code str*/
static void report_insn_error(const char* str){
	const char* msg = concat(insn_error.msg, " `%s'", NULL);
	switch(insn_error.format){
		case ERR_FMT_PLAIN:
			as_bad(msg, str);
			break;
		case ERR_FMT_I:
			as_bad(msg, insn_error.u.i, str);
			break;
		case ERR_FMT_SS:
			as_bad(msg, insn_error.u.ss[0], insn_error.u.ss[1],
					str);
			break;
	}
	free ((char *) msg);
}


struct regname{
	const char* name;
	unsigned int	num;
};

enum reg_file{
	REGF_GPR,
	REGF_SYS,
	REGF_MAX
};

static struct hash_control* reg_names_hash = NULL;


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

void md_begin(void) {
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
	record_alignment(text_section, 2);
	bfd_set_arch_mach(stdoutput, bfd_arch_fusion, 0);

}

static symbolS* make_internal_label(void){
	return (symbolS*) local_symbol_make(FAKE_LABEL_NAME, now_seg,
			(valueT) frag_now_fix(), frag_now);

}

/*pc relative access*/
static void pcrel_access(int destreg, int tempreg, expeessionS *ep,
			const char* lo_insn, const char* lo_patter,
			bfd_reloc_code_real_type hi_reloc,
			bfd_reloc_code_real_type lo_reloc){

	expressionS ep2;
	ep2.X_op = O_symbol;
	ep2.X_add_symbol = make_internal_label();
	ep2.X_add_number = 0;


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

/* output instruction
 * ip: instruction information
 * address_expr: operand of instruciton to be used with reloc_type
 * expansionp: true if instruction is part of macro expansion*/
static void append_insn(struct fusion_cl_insn* ip, expressionS* address_expr,
		bfd_reloc_code_real_type* reloc_type, bfd_bolean expansionp){

	bfd_boolean relaxed_branch = FALSE;
	enum append_method method;
	bfd_boolean relax32;
	int branch_disp;




}


static const char* fusion_ip(char *str, struct fusion_cl_insn* ip, 
		expressionS* imm_expr, bfd_reloc_code_real_type* imm_reloc){
	char* s;
	const char* args;
	char c = 0;
	struct fusion_opc_info_t* insn;
	char* argsStart;
	insigned int regno;



}

void md_assemble(char* str){
	struct fusion_cl_insn insn;
	bfd_reloc_code_real_type unused_reloc[3]= {BFD_RELOC_UNUSED,
			BFD_RELOC_UNUSED, BFD_RELOC_UNUSED};}
		imm_expr.X_op = O_absent;
		offset_expr.X_op = O_absent;
		offset_reloc[0] = BFD_RELOC_UNUSED;
		offset_reloc[1] = BFD_RELOC_UNUSED;
		offset_reloc[2] = BFD_RELOC_UNUSED;

	fusion_assemblin_insn = TRUE;	
	clear_insn_error();
	fusion_ip(str, &insn);
	DBG((_("returned from fusion_ip(%s) insn_word = 0x%x\n"),
			str, insn.insn_word));	
	if(insn_error.msg)
		report_insn_error(str);
	//else if(insn.insn_mo->
	else{
		if(offset_expr.X_op != O_absent)
			append_insn(&insn, &offset_expr, offset_reloc, FALSE);
		else
			append_insn(&insn, NULL, unused_reloc, FALSE);
	}
	fusion_assembling = FALSE;

}

static inline bfd_boolean jmp_reloc_p(bfd_reloc_code_real_type reloc){
	return reloc == BFD_RELOC_FUSION_21_PCREL;
}

static inline bfd_boolean branch_reloc_p(bfd_reloc_code_real_type reloc){
	return reloc == BFD_RELOC_FUSION_14_PCREL;
}

/*pc relative relocations*/
static inline bfd_boolean pcrel_reloc_p(bfd_reloc_code_real_type reloc){
	switch(reloc){
		case BFD_RELOC_FUSION_21_PCREL:
		case BFD_RELOC_FUSION_14_PCREL:
			return TRUE;
		default:
			return FALSE;
	}
}

/* move all labels in 'labels' to current insertion point
 * text_p states whether dealing with text or data segments*/
static void fusion_move_labels(struct insn_label_list* labels, 
		bfd_boolean text_p){
	struct insn_label_list* l;
	valueT val;

	for(l = labels; l != NULL; l = l-> next){
		gas_assert(S_GET_SEGMENT (l->label) == now_seg);
		symbol_set_frag(l->label, frag_now);
		val = (valueT) frag_now_fix();
		S_SET_VALUE(l->label, val);
	}

}

/* move all labels in insn_labels to current insertion point
 * treat them as text labels*/
static void fusion_move_text_labels(void){
	fusion_move_labels(seg_info(now_seg)->label_list, TRUE);
}

/*end current fragment. make it a varient fragment and record the relaxation
 * info*/
//static void relax_close_frag(void){
//}

//static void relax_start(symbolS* symbol){
//	gas_assert(fusion_relax.sequence == 0);
//	fusion_relax.sequence = 1;
//	fusion_relax.symbol = symbol;
//}
//static void relax_end(void){
//	gas_assert(fusion_relax.sequence == 2);
//	relax_close_frag();
//	fusion_relax.sequence = 0;
//}

/*determine if branch*/
static enum branch_p(fuion_cl_insn* ip){
	return(ip->insn_mo->opc & (OPC_BRANCH));
}

/*figure out how to add IP to instruction stream*/
static enum append_method get_append_method(struct fusion_cl_insn* ip,
			expressionS* address_expr, 
			bfd_reloc_code_real_type* reloc_type){
	if(branch_p(ip)){
		return APPEND_ADD_WITH_NOP;
	}

	return APPEND_NORMAL;


}

/* try to resolve relocation, reloc, against constant operand at assembly time
 * return true on success, storing resolved value in result*/
//static bfd_boolean calculate_reloc(bfd_reloc_code_real_type reloc, 
//		offsetT operand, offsetT* result){
//	switch(reloc){
//		case BFD_RELOC_FUSION_14_PCREL:
//			*result = ((operand + 0x8000) >> 14) & 0xffff;
//			return TRUE;
//		case BFD_RELOC_FUSION_21_PCREL:
//			*result = ((operand + 0x80008000ull) >> )
//	}
//}



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

