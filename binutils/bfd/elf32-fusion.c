/* fusion-core support for 32-bit ELF.
   Copyright (C) 2009-2017 Free Software Foundation, Inc.

   Copied from elf32-fr30.c which is..
   Copyright (C) 1998-2017 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "opcode/fusion.h"
#include "elf-bfd.h"
#include "elf/fusion.h"

static reloc_howto_type fusion_elf_howto_table[] = {
	/*Relocation does nothing*/
	HOWTO (R_FUSION_NONE,				/* Type */
			0,							/*rightshift*/
			3,							/*size*/
			0,							/*bit size*/
			FALSE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_dont,		/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_NONE",			/*name*/
			FALSE,						/*partial_inplace*/
			0,							/*src_mask*/
			0,							/*dst_mask*/
			FALSE),						/*pcrel_offset*/

	/*32 bit absolute relocation*/
	HOWTO (R_FUSION_32,					/* Type */
			0,							/*rightshift*/
			2,							/*size*/
			32,							/*bit size*/
			FALSE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_dont,		/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_32",				/*name*/
			FALSE,						/*partial_inplace*/
			0xffffffff,					/*src_mask*/
			0xffffffff,					/*dst_mask*/
			FALSE),						/*pcrel_offset*/

	/*Load Immediate constant relocation*/
	HOWTO (R_FUSION_LI,					/* Type */
			0,							/*rightshift*/
			2,							/*size*/
			32,							/*bit size*/
			FALSE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_dont,		/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_LI",				/*name*/
			FALSE,						/*partial_inplace*/
			GET_IMM_LI(MASK_IMM_LI),	/*src_mask*/
			MASK_IMM_LI,	/*dst_mask*/
			FALSE),						/*pcrel_offset*/
	/*Load Upper Immediate constant relocation*/
	HOWTO (R_FUSION_LUI,				/* Type */
			16,							/*rightshift*/
			2,							/*size*/
			32,							/*bit size*/
			FALSE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_dont,		/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_LUI",				/*name*/
			FALSE,						/*partial_inplace*/
			GET_IMM_LI(MASK_IMM_LI),					/*src_mask*/
			MASK_IMM_LI,	/*dst_mask*/
			FALSE),						/*pcrel_offset*/
	/*Load Immediate PC relative relocation*/
	HOWTO (R_FUSION_LI_PCREL,			/* Type */
			0,							/*rightshift*/
			2,							/*size*/
			32,							/*bit size*/
			FALSE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_dont,		/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_LI_PCREL",		/*name*/
			FALSE,						/*partial_inplace*/
			0x00000000,					/*src_mask*/
			GET_IMM_LI(MASK_IMM_LI),	/*dst_mask*/
			TRUE),						/*pcrel_offset*/
	/*Load Upper Immediate PC relative relocation*/
	HOWTO (R_FUSION_LUI_PCREL,				/* Type */
			0,							/*rightshift*/
			2,							/*size*/
			32,							/*bit size*/
			FALSE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_dont,		/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_LUI_PCREL",				/*name*/
			FALSE,						/*partial_inplace*/
			0x00000000,					/*src_mask*/
			GET_IMM_LI(MASK_IMM_LI),	/*dst_mask*/
			TRUE),						/*pcrel_offset*/
	/*System constant relocation*/
	HOWTO (R_FUSION_SYS,				/* Type */
			0,							/*rightshift*/
			2,							/*size*/
			32,							/*bit size*/
			FALSE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_dont,		/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_SYS",				/*name*/
			FALSE,						/*partial_inplace*/
			0x00000000,					/*src_mask*/
			GET_IMM_SYS(MASK_IMM_SYS),	/*dst_mask*/
			FALSE),						/*pcrel_offset*/
	/*System constant relocation*/
	HOWTO (R_FUSION_I,					/* Type */
			0,							/*rightshift*/
			2,							/*size*/
			32,							/*bit size*/
			FALSE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_dont,		/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_I",				/*name*/
			FALSE,						/*partial_inplace*/
			GET_IMM_I(MASK_IMM_I),		/*src_mask*/
			MASK_IMM_I,					/*dst_mask*/
			FALSE),						/*pcrel_offset*/

	/*Local Symbol relative relocation*/
	HOWTO (R_FUSION_RELATIVE,			/* Type */
			0,							/*rightshift*/
			2,							/*size*/
			32,							/*bit size*/
			TRUE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_dont,		/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_RELATIVE",		/*name*/
			FALSE,						/*partial_inplace*/
			0x00000000,					/*src_mask*/
			0xffffffff,					/*dst_mask*/
			FALSE),						/*pcrel_offset*/


	/*14 Bit Load Relocation*/
	HOWTO (R_FUSION_LOAD,
			0,							/*rightshift*/
			2,							/*size*/
			14,							/*bit size*/
			TRUE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_signed,	/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_LOAD",			/*name*/
			FALSE,						/*partial_inplace*/
			0x00000000,					/*src_mask*/
			GET_IMM_L(MASK_IMM_L),		/*dst_mask*/
			FALSE),						/*pcrel_offset*/
	/*14 Bit Load Relocation*/
	HOWTO (R_FUSION_STORE,
			0,							/*rightshift*/
			2,							/*size*/
			14,							/*bit size*/
			TRUE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_signed,	/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_STORE",			/*name*/
			FALSE,						/*partial_inplace*/
			0x00000000,					/*src_mask*/
			GET_IMM_S(MASK_IMM_S),		/*dst_mask*/
			FALSE),						/*pcrel_offset*/


	/*14 Bit PC Relative Branch*/
	HOWTO (R_FUSION_BRANCH,
			0,							/*rightshift*/
			2,							/*size*/
			32,							/*bit size*/
			TRUE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_signed,	/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_BRANCH",			/*name*/
			FALSE,						/*partial_inplace*/
			0x3fff,					/*src_mask*/
			0x3fff,		/*dst_mask*/
			TRUE),						/*pcrel_offset*/
	/*21 Bit PC Relative Jump*/
	HOWTO (R_FUSION_JUMP,
			0,							/*rightshift*/
			2,							/*size*/
			32,							/*bit size*/
			TRUE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_signed,	/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_JUMP",			/*name*/
			FALSE,						/*partial_inplace*/
			0x001fffff,					/*src_mask*/
			0x001fffff,					/*dst_mask*/
			TRUE),						/*pcrel_offset*/
	/*21 Bit PC Relative Jump Register*/
	HOWTO (R_FUSION_JUMP_O,
			0,							/*rightshift*/
			2,							/*size*/
			32,							/*bit size*/
			FALSE,						/*pc_relative*/
			0,							/*bit position*/
			complain_overflow_signed,	/*complain on overflow*/
			bfd_elf_generic_reloc,		/*special_function*/
			"R_FUSION_JUMP_O",			/*name*/
			FALSE,						/*partial_inplace*/
			0x001fffff,					/*src_mask*/
			0x001fffff,					/*dst_mask*/
			FALSE),						/*pcrel_offset*/
};

struct fusion_reloc_map {
	bfd_reloc_code_real_type bfd_reloc_val;
	unsigned int fusion_reloc_val;
};

static const struct fusion_reloc_map fusion_reloc_map [] = {
	{ BFD_RELOC_NONE,				R_FUSION_NONE},
	{ BFD_RELOC_32,					R_FUSION_32},
	{ BFD_RELOC_16,					R_FUSION_LI},
	{ BFD_RELOC_FUSION_HI16,		R_FUSION_LUI},
	{ BFD_RELOC_16_PCREL,			R_FUSION_LI_PCREL},
	{ BFD_RELOC_HI16_PCREL,			R_FUSION_LUI_PCREL},
	{ BFD_RELOC_8,					R_FUSION_SYS},
	{ BFD_RELOC_FUSION_12, 			R_FUSION_I},
	{ BFD_RELOC_FUSION_STORE,		R_FUSION_STORE},
	{ BFD_RELOC_FUSION_LOAD,		R_FUSION_LOAD},
	{ BFD_RELOC_FUSION_14_PCREL,	R_FUSION_BRANCH},
	{ BFD_RELOC_FUSION_21_PCREL, 	R_FUSION_JUMP},
	{ BFD_RELOC_FUSION_21, 			R_FUSION_JUMP_O},

};

static reloc_howto_type* fusion_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,\
				bfd_reloc_code_real_type code) {

	unsigned int i;

	for(i = sizeof(fusion_reloc_map) / sizeof(fusion_reloc_map[0]); i--; ){
		if(fusion_reloc_map[i].bfd_reloc_val == code)
			return &fusion_elf_howto_table[fusion_reloc_map[i].fusion_reloc_val];
	}
	return NULL;
}

static reloc_howto_type* fusion_reloc_name_lookup( bfd *abfd ATTRIBUTE_UNUSED,\
				const char* r_name) {
	unsigned int i;
	
	for(i = 0; i < sizeof(fusion_elf_howto_table) / sizeof(fusion_elf_howto_table[0]); i++)
		if( (fusion_elf_howto_table[i].name != NULL) && \
			(strcasecmp (fusion_elf_howto_table[i].name, r_name) ==0))
			return &fusion_elf_howto_table[i];
	return NULL;
}

/*set howto pointer for FUSION ELF relocation (?) */
static void fusion_info_to_howto(bfd *abfd ATTRIBUTE_UNUSED,\
								arelent *cache_ptr,\
								Elf_Internal_Rela *dst){
	unsigned int r_type;
	r_type = ELF32_R_TYPE(dst->r_info);
	if(r_type >= (unsigned int) R_FUSION_max){
			/* xgettext:c-format*/	//what is this for?		
			_bfd_error_handler(_("%B: invalid Fusion reloc number: %d"), abfd, r_type);
			r_type = 0;
		}
	cache_ptr->howto = &fusion_elf_howto_table[r_type];


}

static bfd_reloc_status_type fusion_final_link_relocate(reloc_howto_type* howto,
				bfd* input_bfd,
				asection* input_section,
				bfd_byte* contents,
				Elf_Internal_Rela* rel,
				bfd_vma relocation){

	bfd_reloc_status_type r= bfd_reloc_ok;

	switch(howto->type){
		default:
				r = _bfd_final_link_relocate(howto, input_bfd, input_section,
								contents, rel->r_offset,
								relocation, rel->r_addend);
	}
	return r;

}

/*ELF Relocation*/
static bfd_boolean fusion_elf_relocate_section(bfd* output_bfd,
									struct bfd_link_info* info,
									bfd* input_bfd,
									asection* input_section,
									bfd_byte* contents,
									Elf_Internal_Rela* relocs,
									Elf_Internal_Sym* local_syms,
									asection **local_sections){
	Elf_Internal_Shdr* symtab_hdr;
	struct elf_link_hash_entry **sym_hashes;
	Elf_Internal_Rela* rel;
	Elf_Internal_Rela* relend;

	symtab_hdr = &elf_tdata(input_bfd)->symtab_hdr;
	sym_hashes = elf_sym_hashes (input_bfd);
	relend	   = relocs + input_section->reloc_count;

	for(rel = relocs; rel < relend; rel++){
		reloc_howto_type* howto;
		unsigned long r_symndx;
		Elf_Internal_Sym* sym;
		asection* sec;
		struct elf_link_hash_entry* h;
		bfd_vma relocation;
		bfd_reloc_status_type r;
		const char* name;
		int r_type;

		r_type		 = ELF32_R_TYPE(rel->r_info);
		r_symndx	 = ELF32_R_SYM(rel->r_info);
		howto		 = fusion_elf_howto_table + r_type;
		h			 = NULL;
		sym			 = NULL;
		sec			 = NULL;

		if(r_symndx < symtab_hdr->sh_info){
			sym = local_syms + r_symndx;
			sec = local_sections[r_symndx];
			relocation = _bfd_elf_rela_local_sym(output_bfd, sym, &sec, rel);

			name = bfd_elf_string_from_elf_section(input_bfd, symtab_hdr->sh_link, sym->st_name);
			name = (name == NULL) ? bfd_section_name(input_bfd, sec) : name;

		} else {
			bfd_boolean unresolved_reloc, warned, ignored;
			RELOC_FOR_GLOBAL_SYMBOL(info, input_bfd, input_section, rel,
							r_symndx, symtab_hdr, sym_hashes,
							h, sec, relocation, unresolved_reloc,
							warned, ignored);
			name = h->root.root.string;
		}

		if((sec != NULL) && discarded_section(sec))
			RELOC_AGAINST_DISCARDED_SECTION(info, input_bfd, input_section,
							rel, 1, relend, howto, 0, contents);
		if(bfd_link_relocatable(info))
				continue;
		r = fusion_final_link_relocate(howto, input_bfd, input_section,
						contents, rel, relocation);
		if(r != bfd_reloc_ok){
			const char* msg = NULL;

			switch(r){
				case bfd_reloc_overflow:
					(*info->callbacks->reloc_overflow)
				 		(info, (h ? &h-> root : NULL), name, howto->name,
						(bfd_vma) 0, input_bfd, input_section, rel->r_offset);
					break;
				case bfd_reloc_undefined:
					(*info->callbacks->undefined_symbol) (info, name, input_bfd,
									input_section, rel->r_offset, TRUE);
					break;
				case bfd_reloc_outofrange:
					msg = _("internal error: relocation out of range");
					break;
				case bfd_reloc_notsupported:
					msg = _("internal error: unsupported relocation");
					break;
				case bfd_reloc_dangerous:
					msg = _("internal error: dangerous relocation");
					break;
				default:
					msg = _("internal error: who knows what the hell happened, I don't");
					break;
			}
			if(msg)
				(*info->callbacks->warning) (info, msg, name, input_bfd,
								input_section, rel->r_offset);
			
		
		}

	}
	return TRUE;

}

/*Retuning section marked against GC for given relocation
 * copying from Moxie, may alter to resemble riscv??*/
static asection* fusion_elf_gc_mark_hook(asection* sec,
										struct bfd_link_info* info,
										Elf_Internal_Rela* rel,
										struct elf_link_hash_entry* h,
										Elf_Internal_Sym* sym){
	return _bfd_elf_gc_mark_hook(sec, info, rel, h, sym);
}

/*Again copying Moxie;
 * look through relocations for a section during the first
 * phase. no '.got' or '.plt' sections, just need to look
 * at virtual table relocations for gc.*/
static bfd_boolean fusion_elf_check_relocs(bfd* abfd,
										struct bfd_link_info* info,
										asection* sec,
										const Elf_Internal_Rela* relocs){
	Elf_Internal_Shdr* symtab_hdr;
	struct elf_link_hash_entry** sym_hashes;
	const Elf_Internal_Rela* rel;
	const Elf_Internal_Rela* rel_end;

	if(bfd_link_relocatable(info))
			return TRUE;

	symtab_hdr = &elf_tdata(abfd)->symtab_hdr;
	sym_hashes = elf_sym_hashes(abfd);

	rel_end = relocs + sec->reloc_count;
	for(rel = relocs; rel < rel_end; rel++){
		struct elf_link_hash_entry *h;
		unsigned long r_symndx;

		r_symndx = ELF32_R_SYM(rel->r_info);
		if(r_symndx < symtab_hdr->sh_info)
			h = NULL;
		else{
			h = sym_hashes[ r_symndx - (symtab_hdr->sh_info) ];
			while(h->root.type == bfd_link_hash_indirect
				  || h->root.type == bfd_link_hash_warning){
				h = (struct elf_link_hash_entry *)h->root.u.i.link;
			}
			
			h->root.non_ir_ref_regular = 1; //?
		
		}

	}

	return TRUE;

}


/* for static relocations */
//static bfd_reloc_status_type perform_relocation


#define ELF_ARCH			bfd_arch_fusion
#define ELF_MACHINE_CODE	EM_FUSION
#define ELF_MAXPAGESIZE		0x1

#define TARGET_BIG_SYM		fusion_elf32_vec
#define TARGET_BIG_NAME		"elf32-fusion"

#define elf_info_to_howto_rel				NULL
#define elf_info_to_howto					fusion_info_to_howto
#define elf_backend_relocate_section		fusion_elf_relocate_section
#define elf_backend_gc_mark_hook			fusion_elf_gc_mark_hook
#define elf_backend_check_relocs			fusion_elf_check_relocs

#define elf_backend_can_gc_sections			1
#define elf_backend_rela_normal				1

#define bfd_elf32_bfd_reloc_type_lookup		fusion_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		fusion_reloc_name_lookup
//#define bfd_elf32_bfd_reloc_type_lookup 	bfd_default_reloc_type_lookup
//#define bfd_elf32_bfd_reloc_name_lookup		_bfd_norelocs_bfd_reloc_name_lookup
//#define elf_into_to_howto					_bfd_elf_no_info_to_howto

#include "elf32-target.h"
