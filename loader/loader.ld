/* Copyright 2016 FIX94
   This code is licensed to you under the terms of the GNU LGPL, version 3;
   see file LICENSE or http://www.gnu.org/licenses/lgpl-3.0.txt */

OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

SECTIONS {
	. = 0x80001800;

	.start : { start.o(*) }
	.text : { *(.text) }
	.rodata : { *(.rodata .rodata.*)}
	.data : { *(.data) }

	__bss_start = .;
	.bss : { *(.bss) }
	__bss_end = .;

	. = ALIGN(0x40);
	.stack : {
		. += 0x8000;
		_stack_top = .;
	}
}
