//===-- modhi3.S - 16-bit signed remainder for IA-16 ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../assembly.h"

// int __modhi3(int dividend, int divisor);
// See divhi3.S for the original 8086/8088 IDIV compatibility case.

	.p2align 1
DEFINE_COMPILERRT_FUNCTION(__modhi3)
	pushw	%bp
	movw	%sp, %bp
	movw	4(%bp), %ax
	movw	6(%bp), %cx
	cmpw	$0x8000, %ax
	jne	LOCAL_LABEL(divide)
	cmpw	$1, %cx
	jne	LOCAL_LABEL(divide)
	xorw	%ax, %ax
	jmp	LOCAL_LABEL(done)
LOCAL_LABEL(divide):
	cwd
	idivw	%cx
	movw	%dx, %ax
LOCAL_LABEL(done):
	popw	%bp
	retw
END_COMPILERRT_FUNCTION(__modhi3)

NO_EXEC_STACK_DIRECTIVE
