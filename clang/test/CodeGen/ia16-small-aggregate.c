// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o0.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s

struct one_byte {
  unsigned char value;
};

struct three_bytes {
  unsigned char value[3];
};

struct one_byte identity_one(struct one_byte value) { return value; }
struct three_bytes identity_three(struct three_bytes value) { return value; }

extern struct one_byte external_one(struct one_byte);
extern struct three_bytes external_three(struct three_bytes);

struct one_byte call_one(struct one_byte value) { return external_one(value); }
struct three_bytes call_three(struct three_bytes value) {
  return external_three(value);
}

// One-byte aggregates occupy one complete stack word and return through AX.
// CHECK-LABEL: identity_one:
// CHECK:       movb 4(%bp), %al
// CHECK:       movb $0, %ah
// CHECK:       retw

// Three-byte aggregates occupy two stack words and return through DX:AX.
// CHECK-LABEL: identity_three:
// CHECK:       movw 4(%bp), %ax
// CHECK:       movb ({{%[a-z]+}}),
// CHECK:       movb {{.*}}, %dl
// CHECK:       retw

// CHECK-LABEL: call_one:
// CHECK:       movb 4(%bp), %al
// CHECK:       pushw
// CHECK-NEXT:  callw external_one
// CHECK:       addw $2,
// CHECK:       retw

// The high word is pushed first, leaving the aggregate's low-address word at
// the callee's first argument offset.
// CHECK-LABEL: call_three:
// CHECK:       movb ({{%[a-z]+}}), %al
// CHECK:       pushw %ax
// CHECK:       movw 4(%bp),
// CHECK-NEXT:  pushw
// CHECK-NEXT:  callw external_three
// CHECK:       addw $4,
// CHECK:       retw
