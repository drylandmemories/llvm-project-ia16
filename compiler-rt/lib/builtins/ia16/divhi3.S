//===-- divhi3.S - 16-bit signed division for IA-16 ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../assembly.h"

// int __divhi3(int dividend, int divisor);
//
// The original 8086 and 8088 raise divide error when IDIV would produce
// INT16_MIN, even though that result fits in AX. Guard the only defined C
// operation with that quotient, INT16_MIN / 1, before using hardware IDIV.

	.p2align 1
DEFINE_COMPILERRT_FUNCTION(__divhi3)
	pushw	%bp
	movw	%sp, %bp
	movw	4(%bp), %ax
	movw	6(%bp), %cx
	cmpw	$0x8000, %ax
	jne	LOCAL_LABEL(divide)
	cmpw	$1, %cx
	je	LOCAL_LABEL(done)
LOCAL_LABEL(divide):
	cwd
	idivw	%cx
LOCAL_LABEL(done):
	popw	%bp
	retw
END_COMPILERRT_FUNCTION(__divhi3)

NO_EXEC_STACK_DIRECTIVE
