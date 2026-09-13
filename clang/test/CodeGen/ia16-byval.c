// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O1 \
// RUN:   -mllvm -verify-machineinstrs -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O0 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o0.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -O2 \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.o2.o %s
// RUN: %clang_cc1 -triple ia16-unknown-none-elf -target-cpu i8086 -Os \
// RUN:   -mllvm -verify-machineinstrs -emit-obj -o %t.os.o %s

struct five_bytes {
  unsigned char value[5];
};

int endpoints(struct five_bytes value) {
  return value.value[0] + value.value[4];
}

extern int external_endpoints(struct five_bytes);
int call_endpoints(struct five_bytes value) {
  return external_endpoints(value);
}

int mixed_endpoints(int left, struct five_bytes value, int right) {
  return left + value.value[0] + value.value[4] + right;
}

extern int external_mixed(int, struct five_bytes, int);
int call_mixed(int left, struct five_bytes value, int right) {
  return external_mixed(left, value, right);
}

struct five_bytes identity_five(struct five_bytes value) { return value; }

extern struct five_bytes external_five(struct five_bytes);
struct five_bytes call_five(struct five_bytes value) {
  return external_five(value);
}

// The incoming aggregate is inline: its first and last bytes are at BP+4 and
// BP+8 after establishing a near frame pointer.
// CHECK-LABEL: endpoints:
// CHECK:       leaw 4(%bp),
// CHECK:       movb ({{%[a-z]+}}),
// CHECK:       movb 4(%bp),
// CHECK:       retw

// A five-byte aggregate is rounded to three stack words. The partial high
// word is loaded as a byte and zero-extended before it is pushed, followed by
// the remaining words from high address to low address.
// CHECK-LABEL: call_endpoints:
// CHECK:       leaw 4(%bp),
// CHECK:       movb ({{%[a-z]+}}),
// CHECK:       movb $0,
// CHECK:       pushw
// CHECK:       movw ({{%[a-z]+}}),
// CHECK:       pushw
// CHECK:       movw 4(%bp),
// CHECK:       pushw
// CHECK-NEXT:  callw external_endpoints
// CHECK:       addw $6,
// CHECK:       retw

// Scalars surrounding a by-value aggregate retain their sequential incoming
// offsets: left at BP+4, the rounded aggregate at BP+6 through BP+11, and
// right at BP+12.
// CHECK-LABEL: mixed_endpoints:
// CHECK:       movw 4(%bp),
// CHECK:       movw 12(%bp),
// CHECK:       movb 6(%bp),
// CHECK:       leaw 6(%bp),
// CHECK:       retw

// CHECK-LABEL: call_mixed:
// CHECK:       callw external_mixed
// CHECK:       addw $10,
// CHECK:       retw

// A large aggregate return receives its near result pointer first and returns
// that same pointer in AX after copying the inline source at BP+6.
// CHECK-LABEL: identity_five:
// CHECK:       movw 4(%bp),
// CHECK:       leaw 6(%bp),
// CHECK:       movw {{%[a-z]+}}, %ax
// CHECK:       retw

// The nested aggregate call pushes six bytes of by-value data plus its hidden
// near result pointer, and likewise returns the caller's result pointer in AX.
// CHECK-LABEL: call_five:
// CHECK:       callw external_five
// CHECK:       addw $8,
// CHECK:       movw {{%[a-z]+}}, %ax
// CHECK:       retw
