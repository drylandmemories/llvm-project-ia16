// RUN: %clang_cc1 -triple ia16-unknown-none-elf -ffreestanding -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -ffreestanding -mcmodel=huge \
// RUN:   -DIA16_HUGE -emit-llvm -o - %s | FileCheck %s --check-prefix=HUGE

typedef int *near_pointer;
typedef int __attribute__((address_space(1))) *far_pointer;
typedef int __attribute__((address_space(2))) *huge_pointer;
typedef int __attribute__((address_space(3))) *stack_pointer;

near_pointer near_value;
far_pointer far_value;
huge_pointer huge_value;
stack_pointer stack_value;

// CHECK: target datalayout = "e-m:e-p:16:16-p1:32:16:16:16-p2:32:16:16:32-p3:16:16-p4:16:16-p5:32:16:16:16-i32:16-i64:16-f32:16-f64:16-a:0:16-n8:16-S16"
// CHECK: target triple = "ia16-unknown-none-elf"
// CHECK: @near_value = global ptr null, align 2
// CHECK: @far_value = global ptr addrspace(1) null, align 2
// CHECK: @huge_value = global ptr addrspace(2) null, align 2
// CHECK: @stack_value = global ptr addrspace(3) null, align 2
// CHECK: !"ia16-memory-model", !"small"
// CHECK: !"ia16-protected-mode", i32 0

_Static_assert(sizeof(char) == 1, "char");
_Static_assert(sizeof(short) == 2, "short");
_Static_assert(sizeof(int) == 2, "int");
_Static_assert(sizeof(long) == 4, "long");
_Static_assert(sizeof(long long) == 8, "long long");
_Static_assert(sizeof(float) == 4, "float");
_Static_assert(sizeof(double) == 8, "double");
#ifndef IA16_HUGE
_Static_assert(sizeof(void *) == 2, "near pointer");
#endif
_Static_assert(sizeof(far_pointer) == 4, "far pointer");
_Static_assert(sizeof(huge_pointer) == 4, "huge pointer");
_Static_assert(sizeof(stack_pointer) == 2, "stack pointer");

#ifdef IA16_HUGE
_Static_assert(sizeof(void *) == 4, "huge default pointer");
_Static_assert(sizeof(__SIZE_TYPE__) == 4, "huge size_t");
_Static_assert(sizeof(__PTRDIFF_TYPE__) == 4, "huge ptrdiff_t");

// HUGE: @near_value = addrspace(2) global ptr addrspace(2) null, align 2
// HUGE: @far_value = addrspace(2) global ptr addrspace(1) null, align 2
// HUGE: !"ia16-memory-model", !"huge"
#endif
