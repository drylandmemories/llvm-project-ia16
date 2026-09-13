// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o0.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s

extern void signed_byte(signed char value);
extern void unsigned_byte(unsigned char value);

void call_signed_byte(signed char value) { signed_byte(value); }

void call_unsigned_byte(unsigned char value) { unsigned_byte(value); }

int widen_signed_byte(signed char value) { return value; }

int widen_unsigned_byte(unsigned char value) { return value; }

int select_eq_byte(unsigned char value) {
  return value != 0 ? 32767 : -32768;
}

int select_signed_byte(signed char value) {
  return value < 0 ? 1234 : -1234;
}

int select_unsigned_byte(unsigned char value) {
  return value < 7 ? 1234 : -1234;
}

// A signed byte load must preserve its sign before entering a 16-bit ABI slot.
// CHECK-LABEL: call_signed_byte:
// CHECK:       movb 4(%bp),
// CHECK:       movw $128, [[CALL_BIAS:%[a-z]+]]
// CHECK:       xorw [[CALL_BIAS]], [[CALL_VALUE:%[a-z]+]]
// CHECK:       subw [[CALL_BIAS]], [[CALL_VALUE]]
// CHECK:       pushw
// CHECK:       callw signed_byte

// CHECK-LABEL: call_unsigned_byte:
// CHECK:       movb 4(%bp),
// CHECK-NOT:   movw $128,
// CHECK:       pushw
// CHECK:       callw unsigned_byte

// CHECK-LABEL: widen_signed_byte:
// CHECK:       movb 4(%bp),
// CHECK:       movw $128, [[WIDEN_BIAS:%[a-z]+]]
// CHECK:       xorw [[WIDEN_BIAS]], [[WIDEN_VALUE:%[a-z]+]]
// CHECK:       subw [[WIDEN_BIAS]], [[WIDEN_VALUE]]
// CHECK:       retw

// CHECK-LABEL: widen_unsigned_byte:
// CHECK:       movb 4(%bp),
// CHECK-NOT:   movw $128,
// CHECK:       retw

// Byte comparisons feeding a 16-bit select are widened with the signedness
// required by the condition before the target's word comparison.
// CHECK-LABEL: select_eq_byte:
// CHECK:       cmpw
// CHECK:       je
// CHECK:       retw

// CHECK-LABEL: select_signed_byte:
// CHECK:       movw $128,
// CHECK:       xorw
// CHECK:       subw
// CHECK:       cmpw
// CHECK:       jl
// CHECK:       retw

// CHECK-LABEL: select_unsigned_byte:
// CHECK-NOT:   movw $128,
// CHECK:       cmpw
// CHECK:       jb
// CHECK:       retw
