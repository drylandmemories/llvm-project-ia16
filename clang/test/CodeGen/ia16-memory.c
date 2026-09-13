// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o0.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s

void copy_19(unsigned char *destination, const unsigned char *source) {
  __builtin_memcpy(destination, source, 19);
}

void fill_19(unsigned char *destination) {
  __builtin_memset(destination, 0x5a, 19);
}

// CHECK-LABEL: copy_19:
// CHECK:       pushw
// CHECK:       pushw
// CHECK:       pushw
// CHECK:       callw memcpy
// CHECK:       addw $6,
// CHECK:       retw

// The byte fill operand is promoted to a complete 16-bit C argument slot.
// CHECK-LABEL: fill_19:
// CHECK:       movw $90,
// CHECK:       pushw
// CHECK:       callw memset
// CHECK:       addw $6,
// CHECK:       retw
