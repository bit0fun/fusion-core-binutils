# Linker script for MIPS systems.
# Ian Lance Taylor <ian@cygnus.com>.
#
# Copyright (C) 2014-2017 Free Software Foundation, Inc.
# 
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.




test -z "$ENTRY" && ENTRY=__start

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

SECTIONS
{
	${RELOCATING+. = ${TEXT_START_ADDR};}	
	__text_start__ = . ;
	.text : {
		*(.init)
		*(.text)
		${RELOCATING+ __text_end__ = . };
	} ${RELOCATING+ > ram} 
	${RELOCATING+. = ${DATA_ADDR};}
	.rdata :{
		*(.rdata)
	}

	__data_start__ = ALIGN(16) ;
	.data ALIGN(4) : {
		. = ALIGN(4);
		*(.data)
		${CONSTRUCTING+CONSTRUCTORS}

	}
	.sdata : {
		*(.sdata)

	}
	__data_end__ = . ; 
	__bss_start__ = . ; 
	.sbss : {
		*(.sbss)
		*(COMMON)
	}
	.bss : {
		*(.bss)
		*(COMMON)
	}
	__bss_end__ = . ;

	end = .;
}
EOF
