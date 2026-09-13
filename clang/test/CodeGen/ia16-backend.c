// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 -S \
// RUN:   -o - %s | FileCheck %s

int add(int a, int b) { return a + b; }
extern int callee(int, int);
int caller(void) { return callee(2, 3); }
int branch_call(int a, int b) {
  if (a > b)
    return callee(a, 1);
  return callee(b, 2);
}
unsigned char byte_inc(unsigned char value) { return value + 1; }

// CHECK-LABEL: add:
// CHECK:       pushw %bp
// CHECK-NEXT:  movw %sp, %bp
// CHECK-DAG:   movw 6(%bp),
// CHECK-DAG:   movw 4(%bp),
// CHECK:       addw
// CHECK:       popw %bp
// CHECK-NEXT:  retw

// CHECK-LABEL: caller:
// CHECK:       movw $3, %ax
// CHECK-NEXT:  pushw %ax
// CHECK-NEXT:  movw $2, %ax
// CHECK-NEXT:  pushw %ax
// CHECK-NEXT:  callw callee
// CHECK:       addw $4,
// CHECK:       retw

// CHECK-LABEL: branch_call:
// CHECK:       cmpw
// CHECK:       jle
// CHECK:       callw callee
// CHECK:       callw callee
// CHECK:       retw

// CHECK-LABEL: byte_inc:
// CHECK:       movb 4(%bp),
// CHECK:       addb
// CHECK:       xorw
// CHECK:       retw
