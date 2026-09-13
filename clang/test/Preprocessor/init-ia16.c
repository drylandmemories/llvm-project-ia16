// RUN: %clang_cc1 -E -dM -ffreestanding -std=c11 -triple ia16-unknown-none-elf < /dev/null | FileCheck %s --check-prefixes=COMMON,I8086,SMALL
// RUN: %clang_cc1 -E -dM -ffreestanding -std=c11 -triple ia16-pc-dos-elf -target-cpu i186 < /dev/null | FileCheck %s --check-prefixes=COMMON,I186,SMALL
// RUN: %clang_cc1 -E -dM -ffreestanding -std=c11 -triple ia16-pc-dos-elf -target-cpu i286 < /dev/null | FileCheck %s --check-prefixes=COMMON,I286,SMALL
// RUN: %clang_cc1 -E -dM -ffreestanding -std=c11 -triple ia16-pc-dos-elf \
// RUN:   -target-cpu i286 -target-feature +protected-mode < /dev/null | FileCheck %s --check-prefixes=COMMON,I286,SMALL,PROTECTED

// COMMON-DAG: #define __IA16__ 1
// COMMON-DAG: #define __IA16_ABI_VERSION__ 0x000100
// COMMON-DAG: #define __FAR 1
// COMMON-DAG: #define __SEG_SS 1
// COMMON-DAG: #define __STDC_NO_ATOMICS__ 1
// COMMON-DAG: #define __ia16__ 20240218L
// COMMON-DAG: #define __SIZEOF_INT__ 2
// COMMON-DAG: #define __SIZEOF_LONG__ 4
// COMMON-DAG: #define __SIZEOF_LONG_LONG__ 8
// COMMON-DAG: #define __SIZEOF_POINTER__ 2
// COMMON-DAG: #define __SIZEOF_FLOAT__ 4
// COMMON-DAG: #define __SIZEOF_DOUBLE__ 8
// COMMON-DAG: #define __SIZEOF_LONG_DOUBLE__ 8
// COMMON-DAG: #define __SIZE_TYPE__ unsigned int
// COMMON-DAG: #define __PTRDIFF_TYPE__ int
// COMMON-DAG: #define __WCHAR_TYPE__ unsigned int
// COMMON-DAG: #define __IA16_ABI_SEGELF 1
// SMALL-DAG: #define __IA16_CMODEL_SMALL__ 1
// SMALL-DAG: #define __SMALL__ 1

// I8086-DAG: #define _M_IX86 0
// I8086-DAG: #define __IA16_ARCH_I8086 1
// I8086-DAG: #define __IA16_TUNE_I8086 1
// I8086-NOT: __IA16_FEATURE_SHIFT_IMM

// I186-DAG: #define _M_IX86 100
// I186-DAG: #define __IA16_ARCH_I80186 1
// I186-DAG: #define __IA16_FEATURE_ENTER_LEAVE 1
// I186-DAG: #define __IA16_FEATURE_IMUL_IMM 1
// I186-DAG: #define __IA16_FEATURE_PUSHA 1
// I186-DAG: #define __IA16_FEATURE_PUSH_IMM 1
// I186-DAG: #define __IA16_FEATURE_SHIFT_IMM 1
// I186-DAG: #define __IA16_TUNE_I80186 1

// I286-DAG: #define _M_IX86 200
// I286-DAG: #define __IA16_ARCH_I80286 1
// I286-DAG: #define __IA16_FEATURE_80286 1
// I286-DAG: #define __IA16_TUNE_I80286 1

// PROTECTED-DAG: #define __IA16_PROTECTED_MODE__ 1
// PROTECTED-NOT: __IA16_REAL_MODE__
