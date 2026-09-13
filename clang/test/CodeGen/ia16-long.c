// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o0.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s

long identity_long(long value) { return value; }
long constant_long(void) { return 0x12345678L; }
long add_long(long left, long right) { return left + right; }
long subtract_long(long left, long right) { return left - right; }
long multiply_long(long left, long right) { return left * right; }
long divide_long(long left, long right) { return left / right; }
unsigned long divide_unsigned_long(unsigned long left, unsigned long right) {
  return left / right;
}
long remainder_long(long left, long right) { return left % right; }
unsigned long remainder_unsigned_long(unsigned long left,
                                      unsigned long right) {
  return left % right;
}
extern long external_long(long, long);
long call_long(long value) { return external_long(value, 5L); }

// A 32-bit scalar is returned in DX:AX, low word first.
// CHECK-LABEL: identity_long:
// CHECK:       movw 4(%bp), %ax
// CHECK:       movw 6(%bp), %dx
// CHECK:       retw

// CHECK-LABEL: constant_long:
// CHECK:       movw $22136, %ax
// CHECK:       movw $4660, %dx
// CHECK:       retw

// CHECK-LABEL: add_long:
// CHECK:       movw 4(%bp), %ax
// CHECK:       movw 8(%bp),
// CHECK:       addw
// CHECK:       retw

// CHECK-LABEL: subtract_long:
// CHECK:       movw 8(%bp),
// CHECK:       movw 4(%bp), %ax
// CHECK:       subw
// CHECK:       retw

// 8086 multiplication uses AX times a register and returns DX:AX; it never
// relies on the two-operand IMUL introduced by the 80386.
// CHECK-LABEL: multiply_long:
// CHECK-COUNT-3: mulw
// CHECK-NOT:   imulw
// CHECK:       retw

// CHECK-LABEL: divide_long:
// CHECK:       callw __divsi3
// CHECK:       retw

// CHECK-LABEL: divide_unsigned_long:
// CHECK:       callw __udivsi3
// CHECK:       retw

// CHECK-LABEL: remainder_long:
// CHECK:       callw __modsi3
// CHECK:       retw

// CHECK-LABEL: remainder_unsigned_long:
// CHECK:       callw __umodsi3
// CHECK:       retw

// Arguments are pushed right-to-left, with each 32-bit scalar represented as
// high then low pushes so memory at callee entry remains little-endian.
// CHECK-LABEL: call_long:
// CHECK:       movw $0, %ax
// CHECK-NEXT:  pushw %ax
// CHECK:       movw $5, %ax
// CHECK-NEXT:  pushw %ax
// CHECK:       movw 6(%bp), %ax
// CHECK-NEXT:  pushw %ax
// CHECK:       movw 4(%bp), %ax
// CHECK-NEXT:  pushw %ax
// CHECK:       callw external_long
// CHECK:       addw $8,
// CHECK:       retw
