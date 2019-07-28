/* fusion-core ELF support for BFD.
   Copyright (C) 2009-2017 Free Software Foundation, Inc.

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
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _ELF_FUSION_H
#define _ELF_FUSION_H

#include "elf/reloc-macros.h"

/*relocation types*/

START_RELOC_NUMBERS (elf_fusion_reloc_type)
	RELOC_NUMBER (R_FUSION_NONE, 0)
	RELOC_NUMBER (R_FUSION_32, 1)
	RELOC_NUMBER (R_FUSION_LI, 2)
	RELOC_NUMBER (R_FUSION_LUI, 3)
	RELOC_NUMBER (R_FUSION_LI_PCREL, 4)
	RELOC_NUMBER (R_FUSION_LUI_PCREL, 5)
	RELOC_NUMBER (R_FUSION_SYS, 6)
	RELOC_NUMBER (R_FUSION_I, 7)
	RELOC_NUMBER (R_FUSION_RELATIVE, 8)
	RELOC_NUMBER (R_FUSION_LOAD, 9)
	RELOC_NUMBER (R_FUSION_STORE, 10)
	RELOC_NUMBER (R_FUSION_BRANCH, 11)
	RELOC_NUMBER (R_FUSION_JUMP, 12)
	RELOC_NUMBER (R_FUSION_JUMP_O, 13)





END_RELOC_NUMBERS (R_FUSION_max)

#endif /* _ELF_FUSION_H */
