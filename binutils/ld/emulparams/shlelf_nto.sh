SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-shl"
NO_REL_RELOCS=yes
TEXT_START_ADDR=0x08040000
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
ARCH=sh
MACHINE=
TEMPLATE_NAME=elf32
GENERATE_SHLIB_SCRIPT=yes
TEXT_START_SYMBOLS='_btext = .;'
ENTRY=_start
# PR 17739.  Delay checking relocs until after all files have
# been opened and linker garbage collection has taken place.
CHECK_RELOCS_AFTER_OPEN_INPUT=yes
