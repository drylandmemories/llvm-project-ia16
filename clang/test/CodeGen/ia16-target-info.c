// RUN: %clang_cc1 -triple ia16-unknown-none-elf -ffreestanding -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -ffreestanding -mcmodel=huge \
// RUN:   -DIA16_HUGE -emit-llvm -o - %s | FileCheck %s --check-prefix=HUGE
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -ffreestanding -emit-obj \
// RUN:   -o %t.o %s
// RUN: llvm-readelf -n %t.o | FileCheck %s --check-prefix=NOTE
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i80286 \
// RUN:   -target-feature +protected-mode -ffreestanding -emit-obj \
// RUN:   -o %t.protected.o %s 2>&1 | FileCheck %s --allow-empty \
// RUN:   --check-prefix=PROTECTED-DIAG
// RUN: llvm-readelf -n %t.protected.o | FileCheck %s --check-prefix=PROTECTED
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i80286 \
// RUN:   -target-feature -protected-mode -ffreestanding -emit-obj \
// RUN:   -o %t.real.o %s
// RUN: llvm-readelf -n %t.real.o | FileCheck %s --check-prefix=REAL

// NOTE: Displaying notes found in: .note.ia16.abi
// NOTE: Owner
// NOTE-SAME: Data size
// NOTE-SAME: Description
// NOTE: IA16
// NOTE-SAME: NT_VERSION (version)
// NOTE: description data: 49 41 31 36 2d 41 42 49 3a 30 2e 32 00

// PROTECTED-DIAG-NOT: not a recognized feature
// PROTECTED: Displaying notes found in: .note.ia16.mode
// PROTECTED: IA16
// PROTECTED: description data: 49 41 31 36 2d 4d 4f 44 45 3a 70 72 6f 74 65 63 74 65 64 00

// REAL: Displaying notes found in: .note.ia16.mode
// REAL: IA16
// REAL: description data: 49 41 31 36 2d 4d 4f 44 45 3a 72 65 61 6c 00

typedef int *near_pointer;
typedef int __attribute__((address_space(1))) *far_pointer;
typedef int __attribute__((address_space(2))) *huge_pointer;
typedef int __attribute__((address_space(3))) *stack_pointer;

near_pointer near_value;
far_pointer far_value;
huge_pointer huge_value;
stack_pointer stack_value;
long long aligned_long_long;
double aligned_double;

// CHECK: target datalayout = "e-m:e-p:16:16-p1:32:16:16:16-p2:32:16:16:32-p3:16:16-p4:16:16-p5:32:16:16:16-i32:16-i64:16-f32:16-f64:16-a:0:16-n8:16-S16"
// CHECK: target triple = "ia16-unknown-none-elf"
// CHECK: @near_value = global ptr null, align 2
// CHECK: @far_value = global ptr addrspace(1) null, align 2
// CHECK: @huge_value = global ptr addrspace(2) null, align 2
// CHECK: @stack_value = global ptr addrspace(3) null, align 2
// CHECK: @aligned_long_long = global i64 0, align 2
// CHECK: @aligned_double = global double 0.000000e+00, align 2
// CHECK: !"ia16-memory-model", !"small"
// CHECK: !"ia16-protected-mode", i32 0

_Static_assert(sizeof(char) == 1, "char");
_Static_assert(sizeof(short) == 2, "short");
_Static_assert(sizeof(int) == 2, "int");
_Static_assert(sizeof(long) == 4, "long");
_Static_assert(sizeof(long long) == 8, "long long");
_Static_assert(sizeof(float) == 4, "float");
_Static_assert(sizeof(double) == 8, "double");
_Static_assert(_Alignof(long long) == 2, "long long alignment");
_Static_assert(_Alignof(double) == 2, "double alignment");
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
