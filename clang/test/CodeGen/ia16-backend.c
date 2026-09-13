// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 -S \
// RUN:   -o - %s | FileCheck %s

int add(int a, int b) { return a + b; }

// CHECK-LABEL: add:
// CHECK:       pushw %bp
// CHECK-NEXT:  movw %sp, %bp
// CHECK-DAG:   movw 6(%bp),
// CHECK-DAG:   movw 4(%bp),
// CHECK:       addw
// CHECK:       popw %bp
// CHECK-NEXT:  retw
