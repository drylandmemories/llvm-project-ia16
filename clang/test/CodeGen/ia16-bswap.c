// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o0.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8088 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.8088.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i80186 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.80186.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i80188 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.80188.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i80286 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.80286.o %s

unsigned swap_word(unsigned value) { return __builtin_bswap16(value); }

// Every supported CPU implements a word byte swap with the original 8086
// register exchange; no 386 BSWAP or multi-instruction shift sequence is used.
// CHECK-LABEL: swap_word:
// CHECK:       xchgb %ah, %al
// CHECK-NOT:   bswap
// CHECK-NOT:   shl
// CHECK-NOT:   shr
// CHECK:       retw
