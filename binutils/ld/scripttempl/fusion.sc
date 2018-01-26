# Linker script for MIPS systems.
# Ian Lance Taylor <ian@cygnus.com>.
#
# Copyright (C) 2014-2017 Free Software Foundation, Inc.
# 
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.




test -x "$ENTRY" && ENTRY=_start

#if [ -z "$EMBEDDED" ]; then
#	test -z "$TEXT_START_ADDR" && TEXT_START_ADDR = "0x40000"
#else
	test -z "$TEXT_START_ADDR" && TEXT_START_ADDR = "0x40000"
#fi
if  test "x$LD_FLAG" = "xn" -o "x$LD_FLAG" = "xN" ; then
	DATA_ADDR=.
else
	test -z "$DATA_ADDR" && DATA_ADDR=0x20000000
fi
cat <<EOF

 OUTPUT_FORMAT("{OUTPUT_FORMAT}")
 OUTPUT_ARCH(${ARCH})

${LIB_SEARCH_DIRS}

${RELOCATING+ENTRY(${ENTRY})}

#MEMORY
#{
#		rom			: ORIGIN = ${TEXT_START_ADDRESS}		LENGTH = 2G
#		ram			: ORIGIN = ${DATA_ADDR}		LENGTH = 0x7FFF8000

#}



SECTIONS
{
	${RELOCATING+. = ${TEXT_START_ADDR};}	
	.text : {
		${RELOCATING+ _ftext = . };
		*(.init)
		${RELOCATING+ eprol = . };
		*(.text)
		*(.fini)
		${RELOCATING+ etext = . };
		${RELOCATING+ _etext = . };
	} #${RELOCATING+ > ram} 
	. = ${DATA_START};
	${RELOCATING+. = ${DATA_ADDR};}
	.rdata :{
		*(.rdata)
	}
	${RELOCATING+ _fdata = ALIGN(32);}
	.data ALIGN(4) : {
		*(.data)
		. = ALIGN(4);
		${CONSTRUCTING+CONSTRUCTORS}
	}
	.sdata : {
		*(.sdata)
	}
	${RELOCATING+ edata = .;}
	${RELOCATING+ _edata = .;}
	{RELOCATING+ _fbss = .;}
	.sbss : {
		*(.sbss)
		*(COMMON)
	}
	.bss : {
		*(.bss)
		*(COMMON)
	}
	${RELOCATING+ end = .;}
	${RELOCATING+ _end = .;}
}
EOF
