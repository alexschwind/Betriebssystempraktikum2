ENTRY(_start)
SECTIONS
{
	. = 0x00008000;
	.init : { *(.init) }
	.vectors : { *(.vectors) }
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	. = ALIGN(1<<20);
	.data : { *(.data) }
	.bss  : { *(.bss)  }
	.stacks (NOLOAD) : { *(.stacks) }
}
