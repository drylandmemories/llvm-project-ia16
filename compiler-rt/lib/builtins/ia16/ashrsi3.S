//===-- ashrsi3.S - 32-bit arithmetic right shift for IA-16 --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../assembly.h"

// si_int __ashrsi3(si_int value, int count);
// See ashlsi3.S for the IA-16 cdecl word layout.

	.p2align 1
DEFINE_COMPILERRT_FUNCTION(__ashrsi3)
	pushw	%bp
	movw	%sp, %bp
	movw	4(%bp), %ax
	movw	6(%bp), %dx
	movw	8(%bp), %cx
	jcxz	LOCAL_LABEL(done)
LOCAL_LABEL(loop):
	sarw	%dx
	rcrw	%ax
	loop	LOCAL_LABEL(loop)
LOCAL_LABEL(done):
	popw	%bp
	retw
END_COMPILERRT_FUNCTION(__ashrsi3)

NO_EXEC_STACK_DIRECTIVE
