// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s \
// RUN:   --check-prefix=I8086
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i80186 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s \
// RUN:   --check-prefix=I80186
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o0.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8088 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.8088.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i80188 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.80188.o %s

int signed_quotient(int dividend, int divisor) {
  return dividend / divisor;
}

unsigned unsigned_quotient(unsigned dividend, unsigned divisor) {
  return dividend / divisor;
}

int signed_remainder(int dividend, int divisor) {
  return dividend % divisor;
}

unsigned unsigned_remainder(unsigned dividend, unsigned divisor) {
  return dividend % divisor;
}

void signed_both(int dividend, int divisor, int *quotient, int *remainder) {
  *quotient = dividend / divisor;
  *remainder = dividend % divisor;
}

void unsigned_both(unsigned dividend, unsigned divisor, unsigned *quotient,
                   unsigned *remainder) {
  *quotient = dividend / divisor;
  *remainder = dividend % divisor;
}

// I8086-LABEL: signed_quotient:
// I8086:       callw __divhi3
// I8086-NOT:   idivw

// I8086-LABEL: unsigned_quotient:
// I8086:       movw {{.*}}, %ax
// I8086:       movw $0, %dx
// I8086-NEXT:  divw
// I8086-NOT:   callw

// I8086-LABEL: signed_remainder:
// I8086:       callw __modhi3
// I8086-NOT:   idivw

// I8086-LABEL: unsigned_remainder:
// I8086:       movw $0, %dx
// I8086-NEXT:  divw
// I8086:       movw %dx, %ax
// I8086-NOT:   callw

// A quotient/remainder pair reuses one hardware division.
// I8086-LABEL: signed_both:
// I8086:       callw __divhi3
// I8086-NOT:   callw __divhi3
// I8086-NOT:   callw __modhi3
// I8086-NOT:   idivw

// I8086-LABEL: unsigned_both:
// I8086:       movw $0, %dx
// I8086-NEXT:  divw
// I8086-NOT:   divw
// I8086-NOT:   callw

// I80186-LABEL: signed_quotient:
// I80186:       movw {{.*}}, %ax
// I80186:       cwtd
// I80186-NEXT:  idivw
// I80186-NOT:   callw

// I80186-LABEL: unsigned_quotient:
// I80186:       movw $0, %dx
// I80186-NEXT:  divw

// I80186-LABEL: signed_remainder:
// I80186:       cwtd
// I80186-NEXT:  idivw
// I80186:       movw %dx, %ax
// I80186-NOT:   callw

// I80186-LABEL: unsigned_remainder:
// I80186:       movw $0, %dx
// I80186-NEXT:  divw
// I80186:       movw %dx, %ax

// I80186-LABEL: signed_both:
// I80186:       cwtd
// I80186-NEXT:  idivw
// I80186-NOT:   idivw
// I80186-NOT:   callw

// I80186-LABEL: unsigned_both:
// I80186:       movw $0, %dx
// I80186-NEXT:  divw
// I80186-NOT:   divw
// I80186-NOT:   callw
