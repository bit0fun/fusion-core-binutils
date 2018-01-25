# Linker script for MIPS systems.
# Ian Lance Taylor <ian@cygnus.com>.
#
# Copyright (C) 2014-2017 Free Software Foundation, Inc.
# 
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.




test -x "$ENTRY" && ENTRY=_start
test -z "$TEXT_START_ADDR" && TEXT_START_ADDR = "0x40000"
test -z "$DATA_ADDR" && DATA_ADDR= "0x2000000"

cat <<EOF

OUTPUT_FORMAT("{OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})

#${LIB_SEARCH_DIRS}


#MEMORY
#{
#		rom	(rwx)   : ORIGIN = 0x40000		LENGTH = 2G
#		ram	(rw)	: ORIGIN = 0x200000		LENGTH = 0x7FFF8000

#}


${RELOCATING+ENTRY (${ENTRY})}

SECTIONS
{
	.text : {
		*(.init)
		*(.text) *(.text.*) *(.gnu.linkone.t.*)
		*(.strings)
		${RELOCATING+ etext = .};
		${RELOCATING+ _etext = . };
	} 
	${RELOCATING+. = ${DATA_ADDR};}

#	.rdata : {
#		*(.rdata_4) *(.rdata_2) *(.rdata_1) *(.rdata.*) *(.gnu.linkone.r.*) *(.rodata*)
#		${RELOCATING+ _edata = . ; }
#	}
#	${RELOCATING+ _fdata = ALIGN(16);}
	.data : {
		*(.data_4) *(.data_2) *(.data) *(.data.*) *(.gnu.linkonce.d.*)
		${RELOCATING+ _edata = . ; }
	}

	.bss : {
		*(.bss_4) *(.bss_2) *(.bss_1) *(.bss) *(COMMON) *(.bss.*) *(.gnu.linkonce.b.*)
	}
	${RELOCATING+ end = .;}
	${RELOCATING+ _end = .;}
}
EOF
