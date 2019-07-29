/* tc-fusion.h -- Header file for tc-fusion.c.

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

   You should have received a copy of the GNU General Public License along
   with GAS; see the file COPYING.  If not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "opcode/fusion.h"
 

#define TC_FUSION 1
#define TARGET_BYTES_BIG_ENDIAN 0
#define WORKING_DOT_WORD

#define TARGET_FORMAT "elf32-fusion"
#define TARGET_ARCH bfd_arch_fusion

#define md_undefined_symbol(NAME)	0


/* These macros must be defined, but is will be a fatal assembler
   error if we ever hit them.  */
#define md_estimate_size_before_relax(A, B) (as_fatal (_("estimate size\n")), 0)
#define md_convert_frag(B, S, F)            as_fatal (_("convert_frag\n"))

/*PC Relative operands are relative to the start of the opcode,
* and the operand is always one byte into the opcode(?)*/
/*
#define md_pcrel_from(FIX) \
	((FIX)->fx_where + (FIX)->fx_frag->fr_address -1)
*/
extern long md_pcrel_from(struct fix*);
extern void fusion_pop_insert(void);
#define md_pop_insert() fusion_pop_insert()

#define md_section_align(SEGMENT, SIZE) (SIZE)



/* Debug information generation related options */
#define TARGET_USE_CFIPOP 1

#define tc_cfi_frame_initial_instructions fusion_cfi_frame_initial_instructions
extern void fusion_cfi_frame_initial_instructions (void);

#define tc_regname_to_dw2regnum tc_fusion_regname_to_dw2regnum
extern int tc_fusion_regname_to_dw2regnum (char *);

/* Return address register */
#define DWARF2_DEFAULT_RETURN_COLUMN X_RA

/* Alignment for CIE data */
#define DWARF2_CIE_DATA_ALIGNMENT -4

/* Adjust debug line number after relaxation. May not be used right now? */
#define DWARF2_USE_FIXED_ADVANCE_PC 1
