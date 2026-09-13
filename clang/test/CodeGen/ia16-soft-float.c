// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o0.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s

float add_float(float left, float right) { return left + right; }
float subtract_float(float left, float right) { return left - right; }
float multiply_float(float left, float right) { return left * right; }
float divide_float(float left, float right) { return left / right; }
int less_float(float left, float right) { return left < right; }

double add_double(double left, double right) { return left + right; }
double subtract_double(double left, double right) { return left - right; }
double multiply_double(double left, double right) { return left * right; }
double divide_double(double left, double right) { return left / right; }
int less_double(double left, double right) { return left < right; }

double extend_float(float value) { return value; }
float truncate_double(double value) { return value; }

// CHECK-LABEL: add_float:
// CHECK:       callw __addsf3
// CHECK-LABEL: subtract_float:
// CHECK:       callw __subsf3
// CHECK-LABEL: multiply_float:
// CHECK:       callw __mulsf3
// CHECK-LABEL: divide_float:
// CHECK:       callw __divsf3
// CHECK-LABEL: less_float:
// CHECK:       callw __ltsf2

// CHECK-LABEL: add_double:
// CHECK:       callw __adddf3
// CHECK-LABEL: subtract_double:
// CHECK:       callw __subdf3
// CHECK-LABEL: multiply_double:
// CHECK:       callw __muldf3
// CHECK-LABEL: divide_double:
// CHECK:       callw __divdf3
// CHECK-LABEL: less_double:
// CHECK:       callw __ltdf2

// CHECK-LABEL: extend_float:
// CHECK:       callw __extendsfdf2
// CHECK-LABEL: truncate_double:
// CHECK:       callw __truncdfsf2
