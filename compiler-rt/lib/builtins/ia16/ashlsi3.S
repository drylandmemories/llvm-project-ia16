//===-- ashlsi3.S - 32-bit logical left shift for IA-16 ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../assembly.h"

// si_int __ashlsi3(si_int value, int count);
//
// The IA-16 cdecl ABI passes value as low:high words at 4(%bp):6(%bp),
// followed by the 16-bit count at 8(%bp), and returns the result in DX:AX.

	.p2align 1
DEFINE_COMPILERRT_FUNCTION(__ashlsi3)
	pushw	%bp
	movw	%sp, %bp
	movw	4(%bp), %ax
	movw	6(%bp), %dx
	movw	8(%bp), %cx
	jcxz	LOCAL_LABEL(done)
LOCAL_LABEL(loop):
	shlw	%ax
	rclw	%dx
	loop	LOCAL_LABEL(loop)
LOCAL_LABEL(done):
	popw	%bp
	retw
END_COMPILERRT_FUNCTION(__ashlsi3)

NO_EXEC_STACK_DIRECTIVE
