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

float signed_int_to_float(long value) { return value; }
float unsigned_int_to_float(unsigned long value) { return value; }
float signed_long_long_to_float(long long value) { return value; }
float unsigned_long_long_to_float(unsigned long long value) { return value; }
double signed_int_to_double(long value) { return value; }
double unsigned_int_to_double(unsigned long value) { return value; }
double signed_long_long_to_double(long long value) { return value; }
double unsigned_long_long_to_double(unsigned long long value) { return value; }

long float_to_signed_int(float value) { return value; }
unsigned long float_to_unsigned_int(float value) { return value; }
long long float_to_signed_long_long(float value) { return value; }
unsigned long long float_to_unsigned_long_long(float value) { return value; }
long double_to_signed_int(double value) { return value; }
unsigned long double_to_unsigned_int(double value) { return value; }
long long double_to_signed_long_long(double value) { return value; }
unsigned long long double_to_unsigned_long_long(double value) { return value; }

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

// CHECK-LABEL: signed_int_to_float:
// CHECK:       callw __floatsisf
// CHECK-LABEL: unsigned_int_to_float:
// CHECK:       callw __floatunsisf
// CHECK-LABEL: signed_long_long_to_float:
// CHECK:       callw __floatdisf
// CHECK-LABEL: unsigned_long_long_to_float:
// CHECK:       callw __floatundisf
// CHECK-LABEL: signed_int_to_double:
// CHECK:       callw __floatsidf
// CHECK-LABEL: unsigned_int_to_double:
// CHECK:       callw __floatunsidf
// CHECK-LABEL: signed_long_long_to_double:
// CHECK:       callw __floatdidf
// CHECK-LABEL: unsigned_long_long_to_double:
// CHECK:       callw __floatundidf

// CHECK-LABEL: float_to_signed_int:
// CHECK:       callw __fixsfsi
// CHECK-LABEL: float_to_unsigned_int:
// CHECK:       callw __fixunssfsi
// CHECK-LABEL: float_to_signed_long_long:
// CHECK:       callw __fixsfdi
// CHECK-LABEL: float_to_unsigned_long_long:
// CHECK:       callw __fixunssfdi
// CHECK-LABEL: double_to_signed_int:
// CHECK:       callw __fixdfsi
// CHECK-LABEL: double_to_unsigned_int:
// CHECK:       callw __fixunsdfsi
// CHECK-LABEL: double_to_signed_long_long:
// CHECK:       callw __fixdfdi
// CHECK-LABEL: double_to_unsigned_long_long:
// CHECK:       callw __fixunsdfdi
