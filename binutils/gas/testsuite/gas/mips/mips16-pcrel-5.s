	.text

	.space	0x10000

	.ent	foo
	.set	mips16
	.set	noautoextend
foo:
	la	$2, . - 1
	nop
	lw	$2, . - 1
	nop
	la	$2, . - 1 + 0x3fc
	nop
	lw	$2, . - 1 + 0x3fc
	nop
	la	$2, . - 1 + 0x400
	nop
	lw	$2, . - 1 + 0x400
	nop
	la	$2, . - 1 - 0x4
	nop
	lw	$2, . - 1 - 0x4
	nop
	la	$2, . - 1 + 0x7fff
	nop
	lw	$2, . - 1 + 0x7fff
	nop
	la	$2, . - 1 - 0x8000
	nop
	lw	$2, . - 1 - 0x8000
	nop
	la	$2, . - 1 + 0x8000
	nop
	lw	$2, . - 1 + 0x8000
	nop
	la	$2, . - 1 - 0x8001
	nop
	lw	$2, . - 1 - 0x8001
	nop
	la	$2, bar
	nop
	lw	$2, bar
	nop
	.set	autoextend
	.set	nomips16
	.end	foo

	.align	8, 0
	.space	0xfe00

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
