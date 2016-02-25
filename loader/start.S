// Copyright 2016 FIX94
// This code is licensed to you under the terms of the GNU LGPL, version 3;
// see file LICENSE or http://www.gnu.org/licenses/lgpl-3.0.txt

.extern _memset
.extern _main
.globl _start
_start:

	# Disable interrupts.
	mfmsr 3 ; rlwinm 3,3,0,17,15 ; mtmsr 3 ; isync

	# Setup stack.
	lis 1,_stack_top@ha ; addi 1,1,_stack_top@l ; li 0,0 ; stwu 0,-64(1)

	# Clear BSS.
	lis 3,__bss_start@ha ; addi 3,3,__bss_start@l
	li 4,0
	lis 5,__bss_end@ha ; addi 5,5,__bss_end@l ; sub 5,5,3
	bl _memset

	# Go!
	b _main
