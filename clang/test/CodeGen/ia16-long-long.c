// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o0.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s

volatile long long signed_value;
volatile unsigned long long unsigned_value;
volatile int shift_count;

long long multiply(long long rhs) { return signed_value * rhs; }
long long divide(long long rhs) { return signed_value / rhs; }
long long signed_remainder(long long rhs) { return signed_value % rhs; }
unsigned long long unsigned_divide(unsigned long long rhs) {
  return unsigned_value / rhs;
}
unsigned long long unsigned_remainder(unsigned long long rhs) {
  return unsigned_value % rhs;
}
unsigned long long shift_left(void) {
  return unsigned_value << shift_count;
}
unsigned long long shift_right(void) {
  return unsigned_value >> shift_count;
}
long long arithmetic_shift_right(void) {
  return signed_value >> shift_count;
}

// CHECK: callw __muldi3
// CHECK: callw __divdi3
// CHECK: callw __moddi3
// CHECK: callw __udivdi3
// CHECK: callw __umoddi3
