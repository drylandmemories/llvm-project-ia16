// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o0.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s

typedef __builtin_va_list va_list;

int first_int(int marker, ...) {
  va_list ap;
  __builtin_va_start(ap, marker);
  int value = __builtin_va_arg(ap, int);
  __builtin_va_end(ap);
  return value;
}

long first_long(int marker, ...) {
  va_list ap;
  __builtin_va_start(ap, marker);
  long value = __builtin_va_arg(ap, long);
  __builtin_va_end(ap);
  return value;
}

extern long variadic_long(int marker, ...);
long call_variadic(void) { return variadic_long(1, 0x12345678L); }

// The near return address is followed by the fixed marker word, so the first
// unnamed argument starts at entry-stack offset four and BP-relative offset
// six after the frame pointer is established.
// CHECK-LABEL: first_int:
// CHECK:       movw 6(%bp), %ax
// CHECK:       retw

// CHECK-LABEL: first_long:
// CHECK:       leaw 6(%bp),
// CHECK:       addw
// CHECK:       movw ({{%[a-z]+}}), %dx
// CHECK:       movw 6(%bp), %ax
// CHECK:       retw

// A variadic call uses the same right-to-left word layout as an ordinary
// cdecl call, including high-then-low pushes for a 32-bit value.
// CHECK-LABEL: call_variadic:
// CHECK:       movw $4660, %ax
// CHECK-NEXT:  pushw %ax
// CHECK:       movw $22136, %ax
// CHECK-NEXT:  pushw %ax
// CHECK:       movw $1, %ax
// CHECK-NEXT:  pushw %ax
// CHECK:       callw variadic_long
// CHECK:       addw $6,
// CHECK:       retw
